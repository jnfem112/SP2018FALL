import os
import re
import time
import tempfile
import asyncio
import logging
import errno
from enum import Enum

from log_tracker import LogTracker


class PeerDriver:
    def __init__(self, student_id, executable_path, name, peers, logger=None, tmp_dir=None, auto_clean_tmp=True):
        assert name not in peers

        # setup members
        self.tmp_dir = tmp_dir
        self.auto_clean_tmp = auto_clean_tmp
        self.name = name
        self.executable_path = executable_path
        self.socket_path = os.path.join('/tmp', 'mp2-%s.sock' % name)
        self.proc = None
        self.md5_regex = re.compile(br'^[0-9a-fA-F]{32}$')
        self.number_regex = re.compile(br'^[1-9][0-9]*$')
        self.log_tracker = LogTracker()

        # setup logger
        if logger is None:
            self.logger = logging.getLogger('{}_peer'.format(name))
        else:
            self.logger = logger.getChild('{}_peer'.format(name))

        # create repo dir
        if auto_clean_tmp:
            self.repo_dir = tempfile.TemporaryDirectory(dir=tmp_dir)
            self.repo_dir_path = self.repo_dir.name
        else:
            self.repo_dir_path = tempfile.mkdtemp(dir=tmp_dir)

        # create config file
        if auto_clean_tmp:
            self.config_file = tempfile.NamedTemporaryFile(dir=tmp_dir)
            self.config_file_path = self.config_file.name
        else:
            self.config_file_path = tempfile.mktemp(dir=tmp_dir)
            self.config_file = open(self.config_file_path, 'wb')

        name_line = 'name = {}'.format(name)
        peers_line = 'peers = {}'.format(' '.join(peers))
        repo_line = 'repo = {}'.format(self.repo_dir_path)

        self.config_file.write(bytes('\n'.join([name_line, peers_line, repo_line]) + '\n', 'ASCII'))
        self.config_file.flush()

        self.logger.info('Config file %s content', self.config_file_path)
        self.logger.info('----------------')
        self.logger.info(name_line)
        self.logger.info(peers_line)
        self.logger.info(repo_line)
        self.logger.info('----------------')

        # clean socket file
        if os.path.exists(self.socket_path):
            self.logger.warning('Socket file %s already exists, remove it anyway', self.socket_path)
            os.unlink(self.socket_path)

    def __del__(self):
        if self.proc is not None:
            if self.proc.returncode is None:
                try:
                    self.proc.kill()
                except ProcessLookupError:
                    pass

            self.proc = None

        self.config_file.close()

        if self.auto_clean_tmp:
            self.repo_dir.cleanup()

    async def start(self):
        self.logger.debug('Start peer "%s" process', self.name)

        assert self.proc is None, 'Peer process is already started on start()'
        self.proc = await asyncio.create_subprocess_exec(
            'stdbuf',
            '-o0',
            self.executable_path,
            self.config_file_path,
            stdin=asyncio.subprocess.PIPE,
            stdout=asyncio.subprocess.PIPE,
            stderr=None,
        )

        await asyncio.sleep(0.1)

        # check socket file
        if not os.path.exists(self.socket_path):
            return 'Expect socket file {} but not found'.format(self.socket_path)

        return None

    async def kill(self):
        self.logger.debug('Kill peer "%s" process', self.name)

        if self.proc is not None:
            if self.proc.returncode is None:
                try:
                    self.proc.kill()
                except ProcessLookupError:
                    pass

            self.proc = None

        # clean socket file
        if os.path.exists(self.socket_path):
            os.unlink(self.socket_path)

    async def send_exit(self, timeout=30.0):
        assert self.proc is not None, 'Process is not started on send_exit()'

        try:
            command = b'exit\n'
            self.logger.info('stdin: %s', command)

            self.proc.stdin.write(command)
            await self.proc.stdin.drain()
        except ConnectionResetError as err:
            return None, err

        try:
            # wait for process to terminate
            await asyncio.wait_for(self.proc.wait(), timeout)
            self.proc = None

            # clean socket file
            if os.path.exists(self.socket_path):
                self.logger.warning('Socket file %s is not removed when %s peer exits', self.socket_path, self.name)
                os.unlink(self.socket_path)

            return True, None

        except asyncio.TimeoutError:
            return False, 'Peer process does not terminate in {} seconds after exit command'.format(timeout)

    async def send_list(self, timeout=0.55):
        assert self.proc is not None, 'Process is not started on list()'

        try:
            # send command
            command = b'list\n'
            self.logger.info('stdin: %s', command)

            self.proc.stdin.write(command)
            await self.proc.stdin.drain()

            result, err = await self.scan_stdout(timeout=timeout)
            if err is not None:
                return None, err

            return self.parse_list_output(result)

        except ConnectionResetError as err:
            self.logger.debug('%s peer process dies early', self.name)
            return None, err

        return None, None

    async def send_rm(self, fname):
        assert self.proc is not None, 'Process is not started on list()'
        assert fname and fname[0] == ord('@')

        try:
            # send command
            command = b'rm %s\n' % fname
            self.logger.info('stdin: %s', command)

            self.proc.stdin.write(command)
            await self.proc.stdin.drain()

            result, err = await self.scan_stdout(timeout=1.0)
            if err is not None:
                return err

            if result:
                return 'Expect no output from rm command, but get {}'.format(result)

            return None

        except ConnectionResetError as err:
            self.logger.debug('%s peer process dies early', self.name)
            return err

        return None

    async def send_histoy(self, all_option, timeout=0.55):
        assert self.proc is not None, 'Process is not started on send_history()'

        try:
            # send command
            command = b'history -a\n' if all_option else b'history\n'
            self.logger.info('stdin: %s', command)

            self.proc.stdin.write(command)
            await self.proc.stdin.drain()

            result, err = await self.scan_stdout(timeout=timeout)
            if err is not None:
                return None, err

        except ConnectionResetError as err:
            self.logger.debug('%s peer process dies early', self.name)
            return None, err

        return self.parse_history_output(result)

    async def send_cp(self, source, target, timeout=180.0):
        assert self.proc is not None, 'Process is not started on send_cp()'
        assert isinstance(source, bytes) and isinstance(target, bytes)
        assert target[0] == ord('@') or os.path.isdir(os.path.dirname(os.path.realpath(target))), 'Malformed target path'

        try:
            command = b'cp %s %s\n' % (source, target)
            self.logger.info('stdin: %s', command)

            self.proc.stdin.write(command)
            await self.proc.stdin.drain()

            result, err = await self.scan_stdout(timeout=timeout, expect_n_lines=1)
            if err is not None:
                return None, err

        except ConnectionResetError as err:
            self.logger.debug('%s peer process dies early', self.name)
            return None, err

        if len(result) == 1:
            if result[0] == b'success\n':
                return True, None
            elif result[0] == b'fail\n':
                return False, None

        return None, 'Output {} is not understood'.format(result)

    async def send_mv(self, source, target, timeout=180.0):
        assert self.proc is not None, 'Process is not started on send_mv()'
        assert isinstance(source, bytes) and isinstance(target, bytes)

        try:
            command = b'mv %s %s\n' % (source, target)
            self.logger.info('stdin: %s', command)

            self.proc.stdin.write(command)
            await self.proc.stdin.drain()

            result, err = await self.scan_stdout(timeout=timeout, expect_n_lines=1)
            if err is not None:
                return None, err

        except ConnectionResetError as err:
            self.logger.debug('%s peer process dies early', self.name)
            return None, err

        if len(result) == 1:
            if result[0] == b'success\n':
                return True, None
            elif result[0] == b'fail\n':
                return False, None

        return None, 'Output {} is not understood'.format(result)

    async def scan_stdout(self, timeout=3.0, expect_n_lines=None):
        assert self.proc is not None
        assert expect_n_lines is None or isinstance(expect_n_lines, int)

        result = list()
        time_limit = time.time() + timeout

        while time.time() < time_limit and \
              (expect_n_lines is None or len(result) < expect_n_lines):
            try:
                line = await asyncio.wait_for(
                    self.proc.stdout.readline(),
                    max(0.0, time_limit - time.time())
                )
            except asyncio.TimeoutError:
                return result, None

            if not line or line[-1] != ord('\n'):  # EOF case
                if line:
                    self.logger.info('stdout: %s', line)
                    result.append(line)

                self.logger.info('Receiv early EOF from %s peer stdout', self.name)
                return result, 'Receiv early EOF in stdout'

            result.append(line)
            self.logger.info('stdout: %s', line)

        return result, None

    def parse_list_output(self, result):
        # parse list output
        result_iter = iter(result)
        filenames = dict()
        prev_line = None

        for line in result_iter:
            if line[-1] != ord('\n'):
                return None, 'Missing trailing \\n: {}'.format(result)

            if line == b'(MD5)\n':
                break

            if prev_line is not None and prev_line >= line:
                return None, 'Filenames are not ordered correctly: {}'.format(result)

            fname = line[:-1]
            filenames[fname] = None

        for line in result_iter:
            if line[-1] != ord('\n'):
                return None, 'Missing trailing \\n: {}'.format(result)

            tokens = line[:-1].split()
            if len(tokens) != 2:
                return None, 'Malformed list output: {}'.format(result)

            fname, digest = tokens

            if self.md5_regex.match(digest) is None:
                return None, 'Malformed list output: {}'.format(result)

            if fname not in filenames or filenames[fname] is not None:
                return None, 'Malformed list output: {}'.format(result)

            filenames[fname] = digest

        if None in filenames.values():
            return None, 'Malformed list output: {}'.format(result)

        return filenames, None

    def parse_history_output(self, result):
        # return empty list for empty output
        if not result:
            return list(), None

        # define parser states
        class ParserState(Enum):
            EXPECT_COMMIT = 1
            EXPECT_NEW_FILE_SECTION = 2
            NEW_FILE_SECTION = 3
            MODIFIED_SECTION = 4
            COPIED_SECTION = 5
            DELETED_SECTION = 6
            MD5_SECTION = 7
            TIMESTAMP_SECTION = 8
            EXPECT_EOL_OR_BLANK = 9

        commit_list = list()
        state = ParserState.EXPECT_COMMIT
        curr_commit = None

        # utility functions
        def verify_commit(commit):
            if None in commit['new_file'].values() or None in commit['modified'].values():
                return False, 'Some MD5 hash is missing'

            if commit_list and commit_list[-1]['timestamp'] > commit['timestamp']:
                return False, 'Commits are not ordered by timestamp correctly'

            return True, None

        # scan lines
        for line in result:
            # check trailing line break
            if not line or line[-1] != ord('\n'):
                return None, 'Missing trailing \\n: {}'.format(result)

            # state machine parser here
            if state == ParserState.EXPECT_COMMIT:
                assert curr_commit is None

                prefix = line[:9]
                number_literal = line[9:-1]

                if prefix != b'# commit ' or self.number_regex.match(number_literal) is None:
                    return None, 'Expect "# commit N" after blank lines or at beginning, but get "{}": {}'.format(line, result)

                try:
                    index_num = int(number_literal)
                except ValueError:
                    return None, 'Malformed line "{}" in history output: {}'.format(line, result)

                if index_num != len(commit_list) + 1:
                    return None, 'The line "{}" has incorrect index number: {}'.format(line, result)

                state = ParserState.EXPECT_NEW_FILE_SECTION
                curr_commit = {
                    'new_file': dict(),
                    'modified': dict(),
                    'copied': set(),
                    'deleted': set(),
                    'timestamp': None,
                }

            elif state == ParserState.EXPECT_NEW_FILE_SECTION:
                if line == b'[new_file]\n':
                    state = ParserState.NEW_FILE_SECTION
                else:
                    return None, 'Expect [new_file] after "# commit N" line, but get "{}": {}'.format(line, result)

            elif state == ParserState.NEW_FILE_SECTION:
                if line == b'[modified]\n':
                    state = ParserState.MODIFIED_SECTION
                else:
                    fname = line[:-1]
                    if fname in curr_commit['new_file']:
                        return None, 'Duplicated filenames found in [new_file] section'

                    curr_commit['new_file'][fname] = None

            elif state == ParserState.MODIFIED_SECTION:
                if line == b'[copied]\n':
                    state = ParserState.COPIED_SECTION
                else:
                    fname = line[:-1]
                    if fname in curr_commit['modified']:
                        return None, 'Duplicated filenames found in [modified] section'

                    curr_commit['modified'][fname] = None

            elif state == ParserState.COPIED_SECTION:
                if line == b'[deleted]\n':
                    state = ParserState.DELETED_SECTION
                else:
                    tokens = line[:-1].split()
                    if len(tokens) != 3:
                        return None, 'Malformed line in [copied] section: {}'.format(result)

                    from_fname, _, to_fname = tokens
                    entry = (from_fname, to_fname)

                    if entry in curr_commit['copied']:
                        return None, 'Duplicated entry found in [copied] section'

                    curr_commit['copied'].add(entry)

            elif state == ParserState.DELETED_SECTION:
                if line == b'(MD5)\n':
                    state = ParserState.MD5_SECTION
                else:
                    fname = line[:-1]
                    if fname in curr_commit['deleted']:
                        return None, 'Duplicated filenames found in [deleted] section'

                    curr_commit['deleted'].add(fname)

            elif state == ParserState.MD5_SECTION:
                if line == b'(timestamp)\n':

                    # verify every entry in [new_file] and [modified] has a MD5
                    if None in curr_commit['new_file'] or None in curr_commit['modified']:
                        return None, 'Some entry in [new_file] or [modified] does not have corresponding MD5'

                    state = ParserState.TIMESTAMP_SECTION
                else:
                    tokens = line[:-1].split()
                    if len(tokens) != 2:
                        return None, 'Malformed line in (MD5) section: {}'.format(result)

                    fname, digest = tokens
                    if self.md5_regex.match(digest) is None:
                        return None, 'Malformed line in (MD5) section: {}'.format(result)

                    if fname not in curr_commit['new_file'] and fname not in curr_commit['modified']:
                        return None, '{} entry in (MD5) section is not found in [new_file] or [modified] section: {}'.format(fname, result)

                    if fname in curr_commit['new_file']:
                        if curr_commit['new_file'][fname] is None:
                            curr_commit['new_file'][fname] = digest
                        else:
                            return None, 'Duplicated entry found in (MD5) section: {}'.format(result)

                    if fname in curr_commit['modified']:
                        if curr_commit['modified'][fname] is None:
                            curr_commit['modified'][fname] = digest
                        else:
                            return None, 'Duplicated entry found in (MD5) section: {}'.format(result)

            elif state == ParserState.TIMESTAMP_SECTION:
                try:
                    timestamp = int(line[:-1])
                except ValueError:
                    return None, 'Malformed (timestamp) section: {}'.format(result)

                assert curr_commit['timestamp'] is None
                curr_commit['timestamp'] = timestamp
                state = ParserState.EXPECT_EOL_OR_BLANK

            elif state == ParserState.EXPECT_EOL_OR_BLANK:
                if line == b'\n':
                    ok, err = verify_commit(curr_commit)
                    if not ok:
                        return None, err

                    commit_list.append(curr_commit)
                    curr_commit = None
                    state = ParserState.EXPECT_COMMIT
                else:
                    return None, 'Expect a blank line between commits, but get "{}": {}'.format(line, result)

            else:
                raise AssertionError('Unknown state value {}'.format(state))

        if state != ParserState.EXPECT_EOL_OR_BLANK:
            return None, 'Truncated history output: {}'.format(result)

        # process last commit
        ok, err = verify_commit(curr_commit)
        if not ok:
            return None, err

        commit_list.append(curr_commit)

        return commit_list, None

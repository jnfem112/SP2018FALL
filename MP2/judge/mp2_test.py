import os
import asyncio
import random
import time
from enum import Enum
import tempfile
import hashlib
import datetime
import string
from collections import OrderedDict

from peer_driver import PeerDriver
from log_tracker import LogTracker


class Status(Enum):
    OK = 'ok'
    ERROR = 'error'


class Comment(Enum):
    GIT_RESET_ERROR = 'Cannot reset repository'
    BUILD_FAILED = 'Build failed'
    EARLY_BONUS_NOT_REQUESTED = 'Early bonus not requested'
    EXCEPTION_RAISED = 'Exception raised'
    NOT_SUBMITTED = 'Homework is not submitted'
    PASS = 'Pass'
    TEST_FAILED = 'Test failed'
    MULTIPLE = 'Multiple errors'


class TestAction(Enum):
    COMPOUND = 1
    VERIFY_LIST = 2
    VERIFY_HISTORY = 3
    VERIFY_COMBINED_HISTORY = 4
    VERIFY_FILE = 5
    SLEEP = 6
    START_PEER = 7
    RUN_CP = 8
    RUN_MV = 9
    RUN_EXIT = 10
    KILL_PEER = 11
    REGISTER_TEST_FILE = 12
    UNREGISTER_TEST_FILE = 13
    RUN_RM = 14


class Mp2Test:
    def __init__(self, student_id, github_account, repo_dir, logger=None, tmp_dir=None, auto_clean_tmp=True):
        self.student_id = student_id
        self.github_account = github_account
        self.repo_dir = repo_dir
        self.tmp_dir = tmp_dir
        self.auto_clean_tmp = auto_clean_tmp
        self.homework_dir = os.path.join(repo_dir, 'MP2')
        self.executable_path = os.path.join(self.homework_dir, 'loser_peer')
        self.logger = logger.getChild('Mp2Test')
        self.score = None
        self.is_clean_repo = False
        self.peer_name_charset = list(set(string.printable) - set(string.whitespace) - set(string.punctuation))

        os.chdir(repo_dir)

    async def git_clean_and_checkout(self, date):
        os.chdir(self.repo_dir)
        logger = self.logger.getChild('git_clean_and_checkout')

        if date is not None:
            proc = await asyncio.create_subprocess_shell(
                "git reset --hard && git clean -dxf && git checkout $(git log --pretty='format:%H' --before={} -- . | head -n1)".format(date.strftime('%Y-%M-%dT%H:%M')),
                stdin=asyncio.subprocess.PIPE,
                stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE,
            )
        else:
            proc = await asyncio.create_subprocess_shell(
                "git reset --hard && git clean -dxf && git checkout master",
                stdin=asyncio.subprocess.PIPE,
                stdout=asyncio.subprocess.PIPE,
                stderr=asyncio.subprocess.PIPE,
            )

        proc.stdin.close()

        async def log_stdout():
            while True:
                line = await proc.stdout.readline()
                if line:
                    logger.info('git_stdout: %s', line.decode('utf-8'))
                else:
                    return

        async def log_stderr():
            while True:
                line = await proc.stderr.readline()
                if line:
                    logger.info('git_stderr: %s', line.decode('utf-8'))
                else:
                    return

        await asyncio.gather(log_stdout(), log_stderr(), proc.wait())
        if proc.returncode != 0:
            self.is_clean_repo = False
            return False, Comment.GIT_RESET_ERROR, proc

        self.is_clean_repo = True

        try:
            os.chdir(self.homework_dir)
        except FileNotFoundError as err:
            return False, Comment.NOT_SUBMITTED, '{} does not exist'.format(self.homework_dir)

        return True, None, None

    async def build(self):
        assert self.is_clean_repo, 'Call git_reset_and_checkout() before build()'
        logger = self.logger.getChild('git_clean_and_checkout')

        proc = await asyncio.create_subprocess_shell(
            "make -B && [ -f loser_peer ]",
            stdin=asyncio.subprocess.PIPE,
            stdout=asyncio.subprocess.PIPE,
            stderr=asyncio.subprocess.PIPE,
        )

        proc.stdin.close()

        async def log_stdout():
            while True:
                line = await proc.stdout.readline()
                if line:
                    logger.info('make_stdout: %s', line.decode('utf-8'))
                else:
                    return

        async def log_stderr():
            while True:
                line = await proc.stderr.readline()
                if line:
                    logger.info('make_stderr: %s', line.decode('utf-8'))
                else:
                    return

        await asyncio.gather(log_stdout(), log_stderr(), proc.wait())
        return proc.returncode == 0, proc

    async def simple_history_list_test(self, due_date=datetime.datetime(2018, 12, 11, 23, 59)):
        resol_name = '{}-resol'.format(self.student_id)
        all_peers = [resol_name]

        return await self.run_script(
            'simple_history_list_test',
            due_date,
            all_peers,
            [
                (TestAction.REGISTER_TEST_FILE, b'first.bin', None, 4096),
                (TestAction.REGISTER_TEST_FILE, b'second.bin', None, 4096),
                (TestAction.REGISTER_TEST_FILE, b'third.bin', None, 4096),
                (TestAction.START_PEER, all_peers),

                (TestAction.VERIFY_HISTORY, all_peers),
                (TestAction.VERIFY_COMBINED_HISTORY, all_peers),
                (TestAction.VERIFY_LIST, dict(), all_peers),

                (TestAction.RUN_CP, [(resol_name, b'first.bin', b'@a.bin', True)]),
                (TestAction.SLEEP, 3),

                (TestAction.VERIFY_HISTORY, all_peers),
                (TestAction.VERIFY_COMBINED_HISTORY, all_peers),
                (TestAction.VERIFY_LIST, {b'a.bin': b'#first.bin'}, all_peers),

                (TestAction.RUN_CP, [(resol_name, b'@a.bin', b'@b.bin', True)]),
                (TestAction.SLEEP, 3),

                (TestAction.VERIFY_HISTORY, all_peers),
                (TestAction.VERIFY_COMBINED_HISTORY, all_peers),
                (TestAction.VERIFY_LIST, {b'a.bin': b'#first.bin', b'b.bin': b'#first.bin'}, all_peers),

                (TestAction.RUN_CP, [(resol_name, b'second.bin', b'@a.bin', True)]),
                (TestAction.SLEEP, 3),

                (TestAction.VERIFY_HISTORY, all_peers),
                (TestAction.VERIFY_COMBINED_HISTORY, all_peers),
                (TestAction.VERIFY_LIST, {b'a.bin': b'#second.bin', b'b.bin': b'#first.bin'}, all_peers),
            ]
        )

    async def simple_local_repo_test(self, due_date=datetime.datetime(2018, 12, 11, 23, 59)):
        resol_name = '{}-resol'.format(self.student_id)
        all_peers = [resol_name]

        return await self.run_script(
            'simple_local_repo_test',
            due_date,
            all_peers,
            [
                (TestAction.REGISTER_TEST_FILE, b'source.bin', None, 4096),
                (TestAction.START_PEER, all_peers),

                (TestAction.RUN_MV, [(resol_name, b'source.bin', b'@test', True)]),
                (TestAction.SLEEP, 1),

                (TestAction.RUN_CP, [(resol_name, b'@test', b'target.bin', True)]),
                (TestAction.SLEEP, 1),

                (TestAction.VERIFY_FILE, (b'target.bin', b'#source.bin')),
            ]
        )

    async def simple_rm_test(self, due_date=datetime.datetime(2018, 12, 11, 23, 59)):
        resol_name = '{}-resol'.format(self.student_id)
        reep_name = '{}-reep'.format(self.student_id)
        all_peers = [resol_name, reep_name]

        return await self.run_script(
            'simple_rm_test',
            due_date,
            all_peers,
            [
                (TestAction.REGISTER_TEST_FILE, b'source_resol', None, 8192),
                (TestAction.REGISTER_TEST_FILE, b'source_reep', None, 8192),
                (TestAction.START_PEER, all_peers),

                (TestAction.RUN_MV, [(resol_name, b'source_resol', b'@resol.file', True)]),
                (TestAction.SLEEP, 1),

                (TestAction.VERIFY_HISTORY, all_peers),
                (TestAction.VERIFY_COMBINED_HISTORY, all_peers),
                (TestAction.VERIFY_LIST, {b'resol.file': b'#source_resol'}, all_peers),

                (TestAction.RUN_MV, [(reep_name, b'source_reep', b'@reep.file', True)]),
                (TestAction.SLEEP, 1),

                (TestAction.VERIFY_HISTORY, all_peers),
                (TestAction.VERIFY_COMBINED_HISTORY, all_peers),
                (TestAction.VERIFY_LIST, {b'resol.file': b'#source_resol', b'reep.file': b'#source_reep'}, all_peers),

                (TestAction.RUN_RM, [(reep_name, b'@resol.file')]),
                (TestAction.SLEEP, 1),

                (TestAction.VERIFY_HISTORY, all_peers),
                (TestAction.VERIFY_COMBINED_HISTORY, all_peers),
                (TestAction.VERIFY_LIST, {b'reep.file': b'#source_reep'}, all_peers),

                (TestAction.RUN_RM, [(resol_name, b'@reep.file')]),
                (TestAction.SLEEP, 1),

                (TestAction.VERIFY_HISTORY, all_peers),
                (TestAction.VERIFY_COMBINED_HISTORY, all_peers),
                (TestAction.VERIFY_LIST, dict(), all_peers),
            ]
        )

    async def simple_remote_transfer_test(self, due_date=datetime.datetime(2018, 12, 11, 23, 59)):
        resol_name = '{}-resol'.format(self.student_id)
        reep_name = '{}-reep'.format(self.student_id)
        all_peers = [resol_name, reep_name]

        return await self.run_script(
            'simple_remote_transfer_test',
            due_date,
            all_peers,
            [
                (TestAction.REGISTER_TEST_FILE, b'source.bin', None, 4096),
                (TestAction.START_PEER, all_peers),

                (TestAction.RUN_MV, [(resol_name, b'source.bin', b'@test', True)]),
                (TestAction.SLEEP, 1),

                (TestAction.RUN_CP, [(reep_name, b'@test', b'target.bin', True)]),
                (TestAction.SLEEP, 1),

                (TestAction.VERIFY_FILE, (b'target.bin', b'#source.bin')),
            ]
        )

    async def hard_remote_transfer_test(self, due_date=datetime.datetime(2018, 12, 11, 23, 59)):
        n_peers = 5
        peer_names = {n: 'peer_%d' % n for n in range(n_peers)}
        all_peers = list(peer_names.values())

        source_files = list(
            [b'source_%d' % n, random.randint(2 ** 10, 2 ** 14)]
            for n in range(n_peers)
        )
        source_files[0][1] = 2 ** 31  # at least one peer has 2GB file

        register_file_actions = list(
            (TestAction.REGISTER_TEST_FILE, fname, None, size)
            for fname, size in source_files
        )

        first_mv_args = list(
            (peer_names[ind], fname, b'@data_%d' % ind, True)
            for ind, (fname, size) in enumerate(source_files)
        )

        second_cp_args = list(
            (peer_names[ind], b'@data_%d' % ((ind + 1) % n_peers), b'target_%d' % ((ind + 1) % n_peers), True)
            for ind in range(n_peers)
        )

        verify_file_args = list(
            (b'target_%d' % ind, b'#source_%d' % ind)
            for ind, (fname, size) in enumerate(source_files)
        )

        return await self.run_script(
            'hard_remote_transfer_test',
            due_date,
            all_peers,
            [
                (TestAction.COMPOUND, False, *register_file_actions),
                (TestAction.START_PEER, all_peers),

                (TestAction.RUN_MV, first_mv_args),
                (TestAction.SLEEP, 120),

                (TestAction.RUN_CP, second_cp_args),
                (TestAction.SLEEP, 120),

                (TestAction.VERIFY_FILE, *verify_file_args),
            ]
        )

    async def simple_log_update_test(self, due_date=datetime.datetime(2018, 12, 11, 23, 59)):
        resol_name = '{}-resol'.format(self.student_id)
        reep_name = '{}-reep'.format(self.student_id)
        all_peers = [resol_name, reep_name]

        return await self.run_script(
            'simple_log_update_test',
            due_date,
            all_peers,
            [
                (TestAction.REGISTER_TEST_FILE, b'resol_source.bin', None, 4096),
                (TestAction.REGISTER_TEST_FILE, b'reep_source.bin', None, 4096),
                (TestAction.REGISTER_TEST_FILE, b'another_source.bin', None, 4096),
                (TestAction.START_PEER, all_peers),

                (TestAction.VERIFY_HISTORY, all_peers),
                (TestAction.VERIFY_COMBINED_HISTORY, all_peers),
                (TestAction.VERIFY_LIST, dict(), all_peers),

                (TestAction.RUN_CP, [(resol_name, b'resol_source.bin', b'@a.bin', True)]),
                (TestAction.SLEEP, 0.55),

                (TestAction.VERIFY_HISTORY, all_peers),
                (TestAction.VERIFY_COMBINED_HISTORY, all_peers),
                (TestAction.VERIFY_LIST, {b'a.bin': b'#resol_source.bin'}, all_peers),

                (TestAction.RUN_MV, [(reep_name, b'reep_source.bin', b'@b.bin', True)]),
                (TestAction.SLEEP, 0.55),

                (TestAction.VERIFY_HISTORY, all_peers),
                (TestAction.VERIFY_COMBINED_HISTORY, all_peers),
                (TestAction.VERIFY_LIST, {b'a.bin': b'#resol_source.bin', b'b.bin': b'#reep_source.bin'}, all_peers),

                (TestAction.RUN_MV, [(reep_name, b'another_source.bin', b'@b.bin', True)]),
                (TestAction.SLEEP, 0.55),

                (TestAction.VERIFY_HISTORY, all_peers),
                (TestAction.VERIFY_COMBINED_HISTORY, all_peers),
                (TestAction.VERIFY_LIST, {b'a.bin': b'#resol_source.bin', b'b.bin': b'#another_source.bin'}, all_peers),
            ]
        )

    async def hard_log_update_test(self, due_date=datetime.datetime(2018, 12, 11, 23, 59)):
        n_peers = 5

        while True:
            peer_names = {n: 'peer_%d' % n for n in range(n_peers)}
            all_peers = list(peer_names.values())

            if n_peers == len(set(all_peers)):
                break

        source_files = list(
            [b'source_%d' % n, random.randint(2 ** 24, 2 ** 26)]
            for n in range(n_peers)
        )

        register_file_actions = list(
            (TestAction.REGISTER_TEST_FILE, fname, None, size)
            for fname, size in source_files
        )

        first_mv_args = list(
            (peer_names[ind], fname, b'@data_%d_1' % ind, True)
            for ind, (fname, size) in enumerate(source_files)
        )
        first_answer = {
            b'data_%d_1' % ind: b'#%s' % fname
            for ind, (fname, size) in enumerate(source_files)
        }

        second_cp_args = list(
            (peer_names[(ind + n_peers - 1) % n_peers], b'@data_%d_1' % ind, b'@data_%d_2' % ind, True)
            for ind in range(n_peers)
        )
        second_partial_answer = {
            b'data_%d_2' % ind: b'#%s' % fname
            for ind, (fname, size) in enumerate(source_files)
        }
        second_answer = {*first_answer, *second_partial_answer}

        third_mv_args = list(
            (peer_names[(ind + 1) % n_peers], b'@data_%d_2' % ((ind + n_peers - 1) % n_peers), b'@data_%d_1' % ind, True)
            for ind in range(n_peers)
        )
        third_answer = {
            b'data_%d_1' % ((ind + 1) % n_peers): b'#%s' % fname
            for ind, (fname, size) in enumerate(source_files)
        }

        return await self.run_script(
            'hard_log_update_test',
            due_date,
            all_peers,
            [
                (TestAction.COMPOUND, False, *register_file_actions),
                (TestAction.START_PEER, all_peers),

                (TestAction.RUN_MV, first_mv_args),
                (TestAction.SLEEP, 1),
                (TestAction.VERIFY_HISTORY, all_peers),
                (TestAction.VERIFY_COMBINED_HISTORY, all_peers),
                (TestAction.VERIFY_LIST, first_answer, all_peers),

                (TestAction.RUN_CP, second_cp_args),
                (TestAction.SLEEP, 1),
                (TestAction.VERIFY_HISTORY, all_peers),
                (TestAction.VERIFY_COMBINED_HISTORY, all_peers),
                (TestAction.VERIFY_LIST, second_answer, all_peers),

                (TestAction.RUN_MV, third_mv_args),
                (TestAction.SLEEP, 1),
                (TestAction.VERIFY_HISTORY, all_peers),
                (TestAction.VERIFY_COMBINED_HISTORY, all_peers),
                (TestAction.VERIFY_LIST, third_answer, all_peers),
            ]
        )

    async def simple_concurrency_test(self, due_date=datetime.datetime(2018, 12, 11, 23, 59)):
        resol_name = '{}-resol'.format(self.student_id)
        reep_name = '{}-reep'.format(self.student_id)
        all_peers = [resol_name, reep_name]

        return await self.run_script(
            'simple_concurrency_test',
            due_date,
            all_peers,
            [
                (TestAction.REGISTER_TEST_FILE, b'some_big', None, 2 ** 28),
                (TestAction.REGISTER_TEST_FILE, b'some_large', None, 2 ** 28),
                (TestAction.START_PEER, all_peers),
                (TestAction.SLEEP, 0.55),

                (TestAction.RUN_CP, [(resol_name, b'some_big', b'@biiig', True), (reep_name, b'some_large', b'@laaarge', True)]),
                (TestAction.SLEEP, 0.55),

                # Run cp commands on two peers
                # We expect they finish at almost the same time
                (
                    TestAction.COMPOUND,
                    True,
                    (TestAction.RUN_CP, [(resol_name, b'@laaarge', b'another_large', True)]),
                    (TestAction.RUN_CP, [(reep_name, b'@biiig', b'another_big', True)]),
                ),

                (TestAction.VERIFY_FILE, (b'another_big', b'#some_big')),
                (TestAction.VERIFY_FILE, (b'another_large', b'#some_large')),
            ]
        )

    async def simple_restart_test(self, due_date=datetime.datetime(2018, 12, 11, 23, 59)):
        resol_name = '{}-resol'.format(self.student_id)
        reep_name = '{}-reep'.format(self.student_id)
        sf_name = '{}-sf'.format(self.student_id)
        all_peers = [resol_name, reep_name, sf_name]

        list_answer = {
            b'gift': b'#resol_gift',
            b'jewelry': b'#reep_jewelry',
            b'gold': b'#sf_gold',
        }

        return await self.run_script(
            'simple_restart_test',
            due_date,
            all_peers,
            [
                (TestAction.REGISTER_TEST_FILE, b'resol_gift', None, 4096),
                (TestAction.REGISTER_TEST_FILE, b'reep_jewelry', None, 4096),
                (TestAction.REGISTER_TEST_FILE, b'sf_gold', None, 4096),
                (TestAction.START_PEER, all_peers),

                (TestAction.RUN_MV, [(resol_name, b'resol_gift', b'@gift', True)]),
                (TestAction.RUN_MV, [(sf_name, b'sf_gold', b'@gold', True)]),
                (TestAction.SLEEP, 2),

                (TestAction.RUN_EXIT, [(sf_name, False)]), # turn off sf peer
                (TestAction.SLEEP, 2),

                (TestAction.RUN_MV, [(reep_name, b'reep_jewelry', b'@jewelry', True)]),
                (TestAction.SLEEP, 2),

                (TestAction.START_PEER, [sf_name]), # turn on sf peer
                (TestAction.SLEEP, 3),

                (TestAction.VERIFY_HISTORY, all_peers),
                (TestAction.VERIFY_COMBINED_HISTORY, all_peers),
                (TestAction.VERIFY_LIST, list_answer, all_peers),
            ]
        )

    async def run_script(self, test_name, due_date, peer_names, script):
        assert len(peer_names) == len(set(peer_names))

        # prepare test data
        if self.auto_clean_tmp:
            test_dir = tempfile.TemporaryDirectory(dir=self.tmp_dir)
            test_dir_path = bytes(test_dir.name, 'ASCII')
        else:
            test_dir_path = bytes(tempfile.mkdtemp(dir=self.tmp_dir), 'ASCII')

        log_tracker = LogTracker()
        logger = self.logger.getChild(test_name)
        drivers = OrderedDict()
        test_files = dict()

        logger.info('Start %s test', test_name)

        # command processing and utility functions
        async def calc_md5(path):
            md5sum_proc = await asyncio.create_subprocess_exec(
                '/usr/bin/env',
                'md5sum',
                path,
                stdin=None,
                stdout=asyncio.subprocess.PIPE,
                stderr=None,
            )
            data = await md5sum_proc.stdout.read()
            returncode = await md5sum_proc.wait()
            if returncode != 0:
                return None

            assert len(data) > 32
            return data[:32]

        async def create_random_file(path, size):
            dd_proc = await asyncio.create_subprocess_shell(
                b'dd if=/dev/urandom of=%s count=%d bs=8K iflag=count_bytes status=none' % (path, size),
                stdin=None,
                stdout=None,
                stderr=None,
            )

            returncode = await dd_proc.wait()
            assert returncode == 0, 'Failed to create file {} with random content'.format(path)

        async def process_verify_list(args):
            expected_answer, names = args

            # replace test file placeholders with md5 hash
            for name, digest in expected_answer.items():
                assert isinstance(digest, bytes) and len(digest) >= 2

                if digest[0] == ord('#'):
                    test_fname = digest[1:]
                    expected_answer[name] = test_files[test_fname]
                else:
                    assert self.md5_regex(digest)

            # send command
            results_and_errors = await asyncio.gather(
                *(drivers[name].send_list() for name in names)
            )

            for name, (result, error) in zip(names, results_and_errors):
                if error is not None:
                    return Status.OK, Comment.TEST_FAILED, error
                else:
                    logger.debug('Parsed list output from %s: %s', name, result)

                    if result != expected_answer:
                        return Status.OK, Comment.TEST_FAILED, 'Wrong list command output from {}, get {}, but expect {}'.format(name, result, expected_answer)
                    else:
                        logger.info("%s's list command output is correct", name)

        async def process_verify_history(args):
            peer_names = args[0]
            results_and_errors = await asyncio.gather(
                *(drivers[name].send_histoy(False) for name in peer_names)
            )
            for name, (result, error) in zip(peer_names, results_and_errors):
                if error is not None:
                    return Status.OK, Comment.TEST_FAILED, error
                else:
                    logger.debug('Parsed history output from %s: %s', name, result)
                    result_update, error_update = log_tracker.update_history(name, result)
                    if error_update is not None:
                        return Status.OK, Comment.TEST_FAILED, error_update
                    logger.info("%s's history command output is correct", name)

        async def process_verify_combined_history(args):
            peer_names = args[0]

            # verify history -a
            results_and_errors = await asyncio.gather(
                *(drivers[name].send_histoy(True) for name in peer_names)
            )
            for name, (result, error) in zip(peer_names, results_and_errors):
                if error is not None:
                    return Status.OK, Comment.TEST_FAILED, error
                else:
                    logger.debug('Parsed history -a output from %s: %s', name, result)
                    if not log_tracker.verify_combined_history(result):
                        logger.info('Logs are combined incorrectly')
                        return Status.OK, Comment.TEST_FAILED, (
                            'Logs are combined incorrectly, result={}, expected={}'.format(
                                result,
                                log_tracker.get_expected_combined_history(),
                            )
                        )
                    else:
                        logger.info("%s's history -a command output is correct", name)

        async def process_verify_file(args):
            for fname, digest in args:
                if digest is not None:
                    assert isinstance(fname, bytes)
                    assert isinstance(digest, bytes) and len(digest) >= 2

                    path = os.path.join(test_dir_path, fname)

                    # replace filename to exact hash
                    if digest[0] == ord('#'):
                        referred_fname = digest[1:]
                        expected_hash = test_files[referred_fname]
                    else:
                        assert self.md5_regex.match(digest) is not None
                        expected_hash = digest

                    result_hash = await calc_md5(path)
                    if result_hash is None:
                        logger.info('Expect %s file, but it does not exist', path)
                        return Status.OK, Comment.TEST_FAILED, 'Expect {} file, but it does not exist'.format(path)
                    elif result_hash != expected_hash:
                        return Status.OK, Comment.TEST_FAILED, 'Expect file {} has MD5 {}, but get {}'.format(path, expected_hash, result_hash)
                else:
                    if os.path.exists(path):
                        return Status.OK, Comment.TEST_FAILED, "File {} should be deleted, but it's still there."

        async def process_sleep(args):
            sleep_time = args[0]
            await asyncio.sleep(sleep_time)

        async def process_start_peer(args):
            peer_names = args[0]
            results_and_errors = await asyncio.gather(
                *(drivers[name].start() for name in peer_names)
            )
            for name, error in zip(peer_names, results_and_errors):
                if error is not None:
                    return Status.OK, Comment.TEST_FAILED, error

        async def process_kill_peer(args):
            peer_names = args[0]
            await asyncio.gather(
                *(drivers[name].kill() for name in peer_names)
            )

        async def process_run_cp(args):
            cp_args = args[0]

            # replace fnames with real paths
            def augment_arg(cp_arg):
                name, orig_src, orig_dst, allow_failure = cp_arg
                assert isinstance(orig_src, bytes) and isinstance(orig_dst, bytes) and len(orig_src) > 0 and len(orig_dst) > 0

                if orig_src[0] != ord('@'):
                    real_src = os.path.join(test_dir_path, orig_src)
                else:
                    real_src = orig_src

                if orig_dst[0] != ord('@'):
                    real_dst = os.path.join(test_dir_path, orig_dst)
                else:
                    real_dst = orig_dst

                return (name, orig_src, orig_dst, real_src, real_dst, allow_failure)

            augmented_cp_args = list(map(augment_arg, cp_args))

            # run cp command
            results_and_errors = await asyncio.gather(
                *(drivers[name].send_cp(real_src, real_dst) for name, _, _, real_src, real_dst, _ in augmented_cp_args)
            )

            # verify result
            for (name, orig_src, orig_dst, real_src, real_dst, expect), (result, error) in zip(augmented_cp_args, results_and_errors):
                if error is not None:
                    return Status.OK, Comment.TEST_FAILED, error

                if not result:
                    if expect is None:
                        logger.info('Peer %s failed to run cp command, which is accepted by judge', name)
                    elif expect:
                        return Status.OK, Comment.TEST_FAILED, 'Expect peer {} to run cp command sucessfuly, but get failure'.format(name)
                    else:
                        logger.info('Peer %s get failure after cp command, which is accepted by judge', name)
                elif expect is not None and not expect:
                    return Status.OK, Comment.TEST_FAILED, 'Expect peer {} to fail cp command, but get success'.format(name)
                else:
                    if orig_src[0] != ord('@'):
                        if not os.path.exists(real_src):
                            return Status.OK, Comment.TEST_FAILED, 'File {} vanishes after peer {} runs cp command'.format(real_src, name)
                        else:
                            # check if file is illegally touched
                            assert orig_src in test_files
                            source_hash = await calc_md5(real_src)
                            assert source_hash is not None
                            if source_hash != test_files[orig_src]:
                                return Status.OK, Comment.TEST_FAILED, 'Peer {} illegally modifies the file {}'.format(name, real_src)

                    if orig_dst[0] != ord('@'):
                        if not os.path.exists(real_dst):
                            return Status.OK, Comment.TEST_FAILED, 'File {} does not show up after peer {} runs cp command'.format(real_dst, name)
                        else:
                            # update md5
                            digest = await calc_md5(real_dst)
                            assert digest is not None
                            test_files[orig_dst] = digest
                            logger.debug('Automatically register test file %s with hash %s', orig_dst, digest)

        async def process_run_mv(args):
            mv_args = args[0]

            # replace fnames with real paths
            def augment_arg(mv_arg):
                name, orig_src, orig_dst, allow_failure = mv_arg
                assert isinstance(orig_src, bytes) and isinstance(orig_dst, bytes) and len(orig_src) > 0 and len(orig_dst) > 0

                if orig_src[0] != ord('@'):
                    real_src = os.path.join(test_dir_path, orig_src)
                else:
                    real_src = orig_src

                if orig_dst[0] != ord('@'):
                    real_dst = os.path.join(test_dir_path, orig_dst)
                else:
                    real_dst = orig_dst

                return (name, orig_src, orig_dst, real_src, real_dst, allow_failure)

            augmented_mv_args = list(map(augment_arg, mv_args))

            # run mv command
            results_and_errors = await asyncio.gather(
                *(drivers[name].send_mv(real_src, real_dst) for name, _, _, real_src, real_dst, _ in augmented_mv_args)
            )

            # verify result
            for (name, orig_src, orig_dst, real_src, real_dst, expect), (result, error) in zip(augmented_mv_args, results_and_errors):
                if error is not None:
                    return Status.OK, Comment.TEST_FAILED, error

                if not result:
                    if expect is None:
                        logger.info('Peer %s failed to run mv command, which is accepted by judge', name)
                    elif expect:
                        return Status.OK, Comment.TEST_FAILED, 'Expect peer {} to run mv command sucessfuly, but get failure'.format(name)
                    else:
                        logger.info('Peer %s get failure after mv command, which is accepted by judge', name)
                elif expect is not None and not expect:
                    return Status.OK, Comment.TEST_FAILED, 'Expect peer {} to fail mv command, but get success'.format(name)
                else:
                    if orig_src[0] != ord('@'):
                        if os.path.exists(real_src):
                            return Status.OK, Comment.TEST_FAILED, 'File {} is not deleted after peer {} runs mv command'.format(real_src, name)

                    if orig_dst[0] != ord('@'):
                        if not os.path.exists(real_dst):
                            return Status.OK, Comment.TEST_FAILED, 'File {} does not show up after peer {} runs mv'.format(real_dst, name)
                        else:
                            # update md5
                            digest = await calc_md5(real_dst)
                            assert digest is not None
                            test_files[orig_dst] = digest
                            logger.debug('Automatically register test file %s with hash %s', orig_dst, digest)

        async def process_run_rm(args):
            rm_args = args[0]
            results_and_errors = await asyncio.gather(
                *(drivers[peer_name].send_rm(fname) for peer_name, fname in rm_args)
            )

            for (peer_name, fname), error in zip(rm_args, results_and_errors):
                if error is not None:
                    logger.info('Failed to run rm command: %s', error)
                    return Status.OK, Comment.TEST_FAILED, error

        async def process_run_exit(args):
            peer_names = args[0]
            results_and_errors = await asyncio.gather(
                *(drivers[name].send_exit() for name, ignore_failure in peer_names)
            )
            for (name, ignore_failure), (result, error) in zip(peer_names, results_and_errors):
                if error is not None:
                    if not ignore_failure:
                        return Status.OK, Comment.TEST_FAILED, error
                    else:
                        logger.warning('Peer %s get eror on exit command, it is ignored by judge: %s', name, error)
                elif result:
                    logger.info('Peer %s exits successfully', name)
                else:
                    if ignore_failure:
                        logger.warning('Peer %s fails on exit command, it is ignored by judge.', name)
                    else:
                        return Status.OK, Comment.TEST_FAILED, 'Peer {} failed to exit'.format(name)

        async def process_register_test_file(args):
            fname, content, size = args
            path = os.path.join(test_dir_path, fname)

            if content is not None:  # Avoid using this with large content to prevent blocking
                assert isinstance(content, bytes) and size is None
                with open(path, 'wb') as fo:
                    fo.write(content)

                test_files[fname] = await calc_md5(path)
                assert test_files[fname] is not None

            else:
                assert size >= 0
                await create_random_file(path, size)
                test_files[fname] = await calc_md5(path)
                assert test_files[fname] is not None

        async def process_unregister_test_file(args):
            name = args[0]
            path = os.path.join(test_dir_path, name)
            if os.path.exists(path):
                os.unlink(path)
            del test_files[name]

        try:
            # compile
            ok, comment, error = await self.git_clean_and_checkout(due_date)
            if not ok:
                return Status.ERROR, comment, error

            ok, result = await self.build()
            if not ok:
                return Status.ERROR, Comment.BUILD_FAILED, result

            # init peer objects
            for name in peer_names:
                peer = PeerDriver(
                    self.student_id,
                    self.executable_path,
                    name,
                    list(filter(lambda n: n != name, peer_names)),
                    logger=logger,
                    tmp_dir=self.tmp_dir,
                    auto_clean_tmp=self.auto_clean_tmp
                )
                drivers[name] = peer

            # run script
            async def handle_command(command):
                action = command[0]
                args = command[1:]

                async def timed_task_wrapper(coro):
                    begin_time = time.time()
                    ret = await coro
                    timing = time.time() - begin_time
                    return ret, timing

                if action == TestAction.COMPOUND:
                    assert_same_time = args[0]
                    cmd_list = args[1:]
                    assert isinstance(assert_same_time, bool)

                    if assert_same_time:
                        timed_results = await asyncio.gather(
                            *(timed_task_wrapper(handle_command(cmd)) for cmd in cmd_list)
                        )
                        error_results = list(
                            ret for ret, time in timed_results if ret is not None
                        )
                        if error_results:
                            errors = list(ret[2] for ret in error_results)
                            return Status.ERROR, Comment.MULTIPLE, errors

                        timings = list(time for _, time in timed_results)
                        timings.sort()
                        logger.info('Task timings: %s', timings)
                        if timings and timings[-1] - timings[0] > 1.0:
                            return Status.OK, Comment.TEST_FAILED, 'Judge assumes tasks finish at almost the same time, but the meeting failed'
                    else:
                        results = await asyncio.gather(
                            *(handle_command(cmd) for cmd in cmd_list)
                        )
                        error_results = list(filter(lambda ret: ret is not None, results))
                        if error_results:
                            errors = list(ret[2] for ret in error_results)
                            return Status.ERROR, Comment.MULTIPLE, errors
                    return

                if action == TestAction.VERIFY_LIST:
                    ret = await process_verify_list(args)
                elif action == TestAction.VERIFY_HISTORY:
                    ret = await process_verify_history(args)
                elif action == TestAction.VERIFY_COMBINED_HISTORY:
                    ret = await process_verify_combined_history(args)
                elif action == TestAction.VERIFY_FILE:
                    ret = await process_verify_file(args)
                elif action == TestAction.SLEEP:
                    ret = await process_sleep(args)
                elif action == TestAction.START_PEER:
                    ret = await process_start_peer(args)
                elif action == TestAction.RUN_CP:
                    ret = await process_run_cp(args)
                elif action == TestAction.RUN_MV:
                    ret = await process_run_mv(args)
                elif action == TestAction.RUN_EXIT:
                    ret = await process_run_exit(args)
                elif action == TestAction.KILL_PEER:
                    ret = await process_kill_peer(args)
                elif action == TestAction.REGISTER_TEST_FILE:
                    ret = await process_register_test_file(args)
                elif action == TestAction.UNREGISTER_TEST_FILE:
                    ret = await process_unregister_test_file(args)
                elif action == TestAction.RUN_RM:
                    ret = await process_run_rm(args)
                else:
                    assert False, 'Undefined test action {}'.format(action)

                if ret is not None:
                    return ret

            for command in script:
                ret = await handle_command(command)
                if ret is not None:
                    return ret

            return Status.OK, Comment.PASS, None

        finally:
            # finalize
            await asyncio.gather(
                *(peer.kill() for peer in drivers.values())
            )

            if self.auto_clean_tmp:
                test_dir.cleanup()

            logger.info('Finish %s test', test_name)

    def make_random_peer_name(self):
        suffix = ''.join(random.choices(self.peer_name_charset, k=8))
        return '{}-{}'.format(self.student_id, suffix)

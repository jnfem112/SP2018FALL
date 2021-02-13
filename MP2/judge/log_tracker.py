import re
import itertools
from collections import defaultdict

from sortedcontainers import SortedDict


class CombinedLog:
    def __init__(self):
        self.logs = SortedDict()
        self.views = dict()
        self.last_timestamp = None

    def add_commit(self, peer_name, appended_commit):
        timestamp = appended_commit['timestamp']

        if self.last_timestamp is None or self.last_timestamp < timestamp:
            self.last_timestamp = timestamp

        if timestamp in self.logs:
            orig_commit = self.logs[timestamp]
            self.merge_commit(peer_name, orig_commit, appended_commit)
        else:
            logged_commit = dict()

            new_file_dict = dict()
            for fname, digest in appended_commit['new_file'].items():
                new_file_dict[fname] = (digest, peer_name)

            modified_dict = dict()
            for fname, digest in appended_commit['modified'].items():
                modified_dict[fname] = (digest, peer_name)

            copied_dict = dict()
            for occurence in appended_commit['copied']:
                copied_dict[occurence] = peer_name

            deleted_dict = dict()
            for fname in appended_commit['deleted']:
                deleted_dict[fname] = peer_name

            logged_commit = {
                'new_file': new_file_dict,
                'modified': modified_dict,
                'copied': copied_dict,
                'deleted': deleted_dict,
                'timestamp': appended_commit['timestamp'],
            }

            self.logs[timestamp] = logged_commit

        self.replay_from(timestamp)

    def merge_commit(self, peer_name, orig_commit, appended_commit):
        assert orig_commit['timestamp'] == appended_commit['timestamp']

        orig_dict = orig_commit['new_file']
        appended_dict = appended_commit['new_file']
        for fname, new_digest in appended_dict.items():
            if fname not in orig_dict or orig_dict[fname][0] > new_digest:
                orig_dict[fname] = (new_digest, peer_name)

        orig_dict = orig_commit['modified']
        appended_dict = appended_commit['modified']
        for fname, new_digest in appended_dict.items():
            if fname not in orig_dict or orig_dict[fname][0] > new_digest:
                orig_dict[fname] = (new_digest, peer_name)

        orig_dict = orig_commit['copied']
        appended_dict = appended_commit['copied']
        for occurence in appended_dict:
            if occurence not in orig_dict:
                orig_dict[occurence] = peer_name

        orig_dict = orig_commit['deleted']
        appended_dict = appended_commit['deleted']
        for occurence in appended_dict:
            if occurence not in orig_dict:
                orig_dict[occurence] = peer_name

    def replay_from(self, timestamp):
        assert timestamp in self.logs
        lower_ind = self.logs.index(timestamp) - 1

        if lower_ind >= 0:
            lower_timestamp = self.logs.peekitem(lower_ind)[0]
            cur_view = self.views[lower_timestamp]
        else:
            cur_view = dict()

        for stamp in self.logs.irange(timestamp):
            commit = self.logs[stamp]
            cur_view = cur_view.copy()

            # apply copy
            for (from_name, to_name), peer in commit['copied'].items():
                if from_name in cur_view:
                    # workaround to copy tuple
                    digest, peer = cur_view[from_name]
                    cur_view[to_name] = (digest, peer)

            # apply creation
            for fname, (digest, peer) in commit['new_file'].items():
                if fname not in cur_view:
                    cur_view[fname] = (digest, peer)

            # apply modification
            for fname, (digest, peer) in commit['modified'].items():
                if fname in cur_view:
                    cur_view[fname] = (digest, peer)

            # apply deletion
            for fname, peer_name in commit['deleted'].items():
                if fname in cur_view:
                    del cur_view[fname]

            self.views[stamp] = cur_view

    def get_logical_view(self):
        if self.views:
            assert self.last_timestamp is not None
            return {
                fname: digest
                for fname, (digest, peer) in self.views[self.last_timestamp].items()
            }
        else:
            return dict()

    def get_combined_history(self):

        def transform(logged_commit):
            return {
                'new_file': {
                    fname: digest
                    for fname, (digest, peer) in logged_commit['new_file'].items()
                },
                'modified': {
                    fname: digest
                    for fname, (digest, peer) in logged_commit['modified'].items()
                },
                'copied': set(
                    (from_name, to_name) for (from_name, to_name), peer in logged_commit['copied'].items()
                ),
                'deleted': set(
                    fname for fname, peer in logged_commit['deleted'].items()
                ),
                'timestamp': logged_commit['timestamp'],
            }

        return list(map(transform, self.logs.values()))


class LogTracker:
    def __init__(self):
        self.log_table = defaultdict(list)
        self.combined_log = CombinedLog()
        self.md5_regex = re.compile(br'^[0-9a-fA-F]{32}$')

    def update_history(self, name, history):
        log = self.log_table[name]

        # compare old commits
        if len(history) < len(log):
            return False, 'New history has less commits than earilier history, current={}, earlier={}'.format(
                history,
                log,
            )

        if history[:len(log)] != log:
            return False, 'New history diverses from earlier history, current={}, earlier={}'.format(
                history,
                log,
            )

        # utility function
        def md5_verifier(s):
            return self.md5_regex.match(s) is not None

        # add new commits
        prev_commit = log[-1] if log else None
        for commit in history[len(log):]:
            assert 'timestamp' in commit
            assert 'new_file' in commit
            assert 'modified' in commit
            assert 'copied' in commit
            assert 'deleted' in commit

            # validate commit correctness
            if not (commit['new_file'] or commit['modified'] or commit['copied'] or commit['deleted']):
                return False,'Empty commit detected'

            hash_values = itertools.chain(commit['new_file'].values(), commit['modified'].values())
            if not all(map(md5_verifier, hash_values)):
                return False, 'Invalid MD5 hash format'

            if prev_commit is not None and prev_commit['timestamp'] > commit['timestamp']:
                return False,  'Timestamp is not correct'

            # save commit
            log.append(commit)
            self.combined_log.add_commit(name, commit)

            prev_commit = commit

        return True, None

    def get_expected_logical_view(self):
        return self.combined_log.get_logical_view()

    def get_expected_history(self, name):
        return self.log_table[name]

    def get_expected_combined_history(self):
        return self.combined_log.get_combined_history()

    def verify_logical_view(self, proposed_view):
        expected_view = self.combined_log.get_logical_view()
        return expected_view == proposed_view

    def verify_combined_history(self, proposed_history):
        expected_history = self.combined_log.get_combined_history()
        return proposed_history == expected_history

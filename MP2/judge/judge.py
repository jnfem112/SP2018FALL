#!/usr/bin/env python3
import os
import csv
import argparse
import asyncio
import logging
import pathlib
import traceback
import datetime

import coloredlogs
from mp2_test import Mp2Test, Status, Comment


async def main():
    # compute default paths
    default_tmp_dir = os.path.join(
        os.path.dirname(os.path.realpath(__file__)),
        'judge_tmp'
    )

    default_log_dir = os.path.join(
        os.path.dirname(os.path.realpath(__file__)),
        'judge_log'
    )

    # parse arguments
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('ACCOUNT_FILE')
    arg_parser.add_argument('REPO_DIR')
    arg_parser.add_argument('--log-dir', default=default_log_dir)
    arg_parser.add_argument('--log-level', default='INFO')
    arg_parser.add_argument('--tmp-dir', default=default_tmp_dir)
    arg_parser.add_argument('--keep-tmp-files', action='store_true')
    arg_parser.add_argument('--due-date', default='2018-12-11T23:59')
    args = arg_parser.parse_args()

    # make sure temp dir exists
    if args.tmp_dir is not None:
        pathlib.Path(args.tmp_dir).mkdir(parents=True, exist_ok=True)

    # setup logging
    pathlib.Path(args.log_dir).mkdir(parents=True, exist_ok=True)
    log_file = os.path.join(os.path.realpath(args.log_dir), 'log.txt')
    logger = logging.getLogger('main')
    logger.setLevel(args.log_level)

    formatter = logging.Formatter('[%(levelname)s] (%(asctime)s) %(name)s: %(message)s', datefmt='%Y-%m-%d %H:%M:%S')
    file_handler = logging.FileHandler(log_file, mode='w', encoding='UTF-8')
    file_handler.setFormatter(formatter)
    logger.addHandler(file_handler)

    with open(args.ACCOUNT_FILE, 'r') as file_account:
        reader = csv.reader(file_account)
        account_list = list(map(lambda row: (row[0].upper(), row[1]), reader))

    # start process pool
    due_date = datetime.datetime.fromisoformat(args.due_date)

    def make_judge_corotine(row):
        student_id = row[0]
        github_account = row[1]
        repo_dir =  os.path.join(
            os.path.realpath(args.REPO_DIR),
            ''.format(row[0]),
        )
        log_dir = os.path.realpath(args.log_dir)
        return judge((student_id, github_account, repo_dir, log_dir, args.log_level, args.tmp_dir, due_date, not args.keep_tmp_files))

    corotine_gen = map(make_judge_corotine, account_list)

    await asyncio.gather(*corotine_gen)


async def judge(args):
    student_id, github_account, repo_dir, log_dir, log_level, tmp_dir, due_date, auto_clean_tmp = args

    # setup logger
    log_file = os.path.join(log_dir, '{}.log'.format(student_id))
    logger = logging.getLogger(student_id)
    formatter = logging.Formatter('[%(levelname)s]\t(%(asctime)s)\t%(name)s:\t%(message)s', datefmt='%Y-%m-%d %H:%M:%S')
    file_handler = logging.FileHandler(log_file, mode='w', encoding='UTF-8')
    file_handler.setFormatter(formatter)
    logger.addHandler(file_handler)
    coloredlogs.install(level=log_level)

    logger.debug('Start judging %s', student_id)

    context = {
        'status': None,
        'comment': dict(),
        'error': dict(),
        'score': 0,
    }

    try:
        tester = Mp2Test(student_id, github_account, repo_dir, logger=logger, tmp_dir=tmp_dir, auto_clean_tmp=auto_clean_tmp)

        # test history and list copy
        test_name = 'simple_history_list_test'
        status, comment, error = await tester.simple_history_list_test(due_date=due_date)
        context['comment'][test_name] = comment.value
        context['error'][test_name] = error

        if status != Status.OK:
            context['status'] = status
            return
        else:
            assert status == Status.OK
            if comment == Comment.PASS:
                context['score'] += 3

        # test local/repo transfer
        test_name = 'simple_local_repo_test'
        status, comment, error = await tester.simple_local_repo_test(due_date=due_date)
        context['comment'][test_name] = comment.value
        context['error'][test_name] = error

        if status == Status.ERROR:
            context['status'] = Status.ERROR
            return
        else:
            assert status == Status.OK
            if comment == Comment.PASS:
                context['score'] += 1

        # test remote file transfer
        test_name = 'simple_remote_transfer_test'
        status, comment, error = await tester.simple_remote_transfer_test(due_date=due_date)
        context['comment'][test_name] = comment.value
        context['error'][test_name] = error

        if status != Status.OK:
            context['status'] = status
            return
        else:
            assert status == Status.OK
            pass_simple = comment == Comment.PASS
            if pass_simple:
                context['score'] += 0.5

        if pass_simple:
            # test large file remove transfer
            test_name = 'hard_remote_transfer_test'
            status, comment, error = await tester.hard_remote_transfer_test(due_date=due_date)
            context['comment'][test_name] = comment.value
            context['error'][test_name] = error

            if status != Status.OK:
                context['status'] = status
                return
            else:
                assert status == Status.OK
                if comment == Comment.PASS:
                    context['score'] += 0.5
        else:
            logger.info('The program fails simple_remote_transfer_test, skip harder hard_remote_transfer_test.')

        # check if history/list is updated in 0.5 seconds
        test_name = 'simple_log_update_test'
        status, comment, error = await tester.simple_log_update_test(due_date=due_date)
        context['comment'][test_name] = comment.value
        context['error'][test_name] = error

        if status == Status.ERROR:
            context['status'] = Status.ERROR
            return
        else:
            assert status == Status.OK
            pass_simple = comment == Comment.PASS
            if pass_simple:
                context['score'] += 0.5

        if pass_simple:
            # test harder log update
            test_name = 'hard_log_update_test'
            status, comment, error = await tester.simple_log_update_test(due_date=due_date)
            context['comment'][test_name] = comment.value
            context['error'][test_name] = error

            if status != Status.OK:
                context['status'] = status
                return
            else:
                assert status == Status.OK
                if comment == Comment.PASS:
                    context['score'] += 0.5
        else:
            logger.info('The program fails simple_log_update_test, skip harder hard_log_update_test.')

        # test if rm command is implemented correctly
        test_name = 'simple_rm_test'
        status, comment, error = await tester.simple_rm_test(due_date=due_date)
        context['comment'][test_name] = comment.value
        context['error'][test_name] = error

        if status == Status.ERROR:
            context['status'] = Status.ERROR
            return
        else:
            assert status == Status.OK
            if comment == Comment.PASS:
                context['score'] += 1.0

        # test if two peers don't block each other
        test_name = 'simple_concurrency_test'
        status, comment, error = await tester.simple_concurrency_test(due_date=due_date)
        context['comment'][test_name] = comment.value
        context['error'][test_name] = error

        if status == Status.ERROR:
            context['status'] = Status.ERROR
            return
        else:
            assert status == Status.OK
            if comment == Comment.PASS:
                context['score'] += 2.0

        # test if peer restarts properly
        test_name = 'simple_restart_test'
        status, comment, error = await tester.simple_restart_test(due_date=due_date)
        context['comment'][test_name] = comment.value
        context['error'][test_name] = error

        if status == Status.ERROR:
            context['status'] = Status.ERROR
            return
        else:
            assert status == Status.OK
            if comment == Comment.PASS:
                context['score'] += 1.0

        # all test pass if reaching here
        context['status'] = Status.OK

    except Exception as err:
        context['status'] = Status.ERROR
        context['comment'] = Comment.EXCEPTION_RAISED.value
        context['error'] = err
        context['score'] = None
        traceback.print_exc()
    finally:
        # assersions
        assert context['status'] is not None
        assert context['comment'] is not None

        # scale score
        if context['score'] is not None:
            context['score'] *= 1.5

        # write log
        logger.info('status=%s', context['status'].value)
        logger.info('comment=%s', context['comment'])
        logger.info('error=%s', context['error'])
        logger.info('score=%s', context['score'])


if __name__ == '__main__':
    asyncio.run(main())


import argparse
import os

from pathlib import Path
from typing import List, Optional

from git import Repo

from FileErrorDatabase import RepoFile, Repository, Repositories, FileErrorDatabase


ExtensionsOfInterest = ['*.OBK', '*.OLB', '*.DSN', '*.DBK']

def get_files_of_interest(path: str, file_extensions: List[str]) -> List[str]:
    """
    Get a list of absolute paths to files with specified extension.
    This function searches recursively through the path.

    Usage e.g.
        >>> files = get_files_of_interest('/home/some_dir', ['*.txt', '*.log'])
        >>> print(files)
        >>> ['/home/som_dir/sub_dir/foo.txt', '/home/some_dir/bar.log']
    """

    files = []

    for extension in file_extensions:
        for file in Path(path).rglob(extension):
            files += [file]

    return files

def add_new_repo_to_database(db: FileErrorDatabase, url: str, base_path: str) -> None:

    author  = url.split('.git')[0].split('/')[-2]
    project = url.split('.git')[0].split('/')[-1]

    base_path = os.path.join(base_path, author, project)

    exists_on_fs = os.path.exists(base_path)

    if exists_on_fs:
        print(f'Repository {base_path} does already exist, do not clone.')
        fs_repo = Repo(base_path)
    else:
        try:
            fs_repo = Repo.clone_from(url, base_path)
        except:
            raise Exception(f'Failed cloning repository from `{url}`!')

    commit = fs_repo.head.commit

    repo = Repository(author=author, commit=str(commit), files=[], project=project, url=url)

    files = get_files_of_interest(base_path, ExtensionsOfInterest)

    for file in files:
        file = os.path.join('.', os.path.relpath(file, base_path))
        repo_file = RepoFile(path=file, errors=99999)

        if not db.isFileInRepo(repo, repo_file):
            db.addFileToRepo(repo, repo_file)

    db.addRepo(repo)

def write_back_err_count(db: FileErrorDatabase, path_err_cnt_log: str, path_thirdparty_designs: str) -> None:
    with open(path_err_cnt_log, 'r') as f:
        log = f.readlines()

    for line in log:
        regex = "^Expected \s*(\d+)\s* errors and got \s*(\d+)\s* errors in `(.*)`$"

        matches = re.findall(regex, line)[0]

        expect_err = int(matches[0])
        actual_err = int(matches[1])
        file_path  = str(matches[2])

        author  = file_path.split(os.path.sep)[2]
        project = file_path.split(os.path.sep)[3]

        path_repo = os.path.join(path_thirdparty_designs, author, project)

        # Make path relative
        file_path = os.path.relpath(file_path, path_repo)
        file_path = os.path.join('.', file_path)

        repo = db.getRepoByNames(author, project)
        file = db.getFileByPath(repo, file_path)

        db.updateErrorCounter(file, actual_err)


def create_repo_cpp_unit_test(repo: Repository, path_repo: str, path_unit_test: str) -> None:
    with open(path_unit_test, 'w') as f:
        preamble = """// THIS FILE IS AUTOMATICALLY GENERATED! DO NOT MODIFY IT!


#include <filesystem>

#include <catch2/catch.hpp>

#include <Parser.hpp>

#include "Helper.hpp"


namespace fs = std::filesystem;"""

        f.write(preamble)

        for file in repo.files:
            file_name = os.path.basename(file.path)
            full_path = os.path.normpath(os.path.join(path_repo, file.path))
            unit_test = f"""


TEST_CASE("{repo.author} - {repo.project} - {file_name} : Check File {full_path}", "[Misc]")
{{
    const fs::path inputFile{{"{full_path}"}};

    Parser parser{{inputFile}};
    Library lib = parser.parseLibrary();
    check_error_count(inputFile, parser.getFileErrCtr(), {file.errors});
    REQUIRE(parser.getFileErrCtr() <= {file.errors});
}}"""

            f.write(unit_test)


parser = argparse.ArgumentParser()

parser.add_argument('-r', '--download_repos',
                    default=False,
                    dest='do_download',
                    help='True if yout want to Download and Checkout the Repositories',
                    type=bool
                    )

parser.add_argument('-g', '--generate_unit_tests',
                    default=False,
                    dest='generate_unit_tests',
                    help='True if you want to Generate Unit Test Files',
                    type=bool
                    )

parser.add_argument('-w', '--write_back_error_count',
                    default=False,
                    dest='write_back',
                    help='True if you want to Write Back the Error Count to the Database',
                    type=bool
                    )

parser.add_argument('-d', '--database_file_path',
                    default='repos.yaml',
                    dest='path_db',
                    help='Path to the Database File',
                    type=str
                    )

parser.add_argument('-u', '--unit_test_path',
                    default=os.path.join('test', 'src', 'Misc'),
                    dest='path_unit_tests',
                    help='Path to the Unit Test Directory',
                    type=str
                    )

parser.add_argument('-t', '--third_party_path',
                    default='thirdparty',
                    dest='path_thirdparty_designs',
                    help='Path to the Third Party Design Directory',
                    type=str
                    )

parser.add_argument('-e', '--err_cnt_log_path',
                    default=os.path.join('test', 'test_err_cnt.log'),
                    dest='path_err_cnt_log',
                    help='Path to the Error Counter Log File',
                    type=str
                    )

args = parser.parse_args()


if __name__ == '__main__':

    do_generate_unit_tests = args.generate_unit_tests
    do_write_back = args.write_back
    do_download = args.do_download

    path_thirdparty_designs = args.path_thirdparty_designs
    path_unit_tests         = args.path_unit_tests
    path_err_cnt_log        = args.path_err_cnt_log

    path_db = args.path_db

    if not do_generate_unit_tests and not do_write_back and not do_download:
        parser.print_help()

    def generate_unit_tests(db: FileErrorDatabase, path_thirdparty_designs: str, path_unit_tests: str) -> None:
        for repo in db.data.repositories:
            path_repo      = os.path.join(path_thirdparty_designs, repo.author, repo.project)
            path_unit_test = os.path.join(path_unit_tests, f'Test_{repo.author}_{repo.project}.cpp')

            create_repo_cpp_unit_test(repo, path_repo, path_unit_test)

    db = FileErrorDatabase()
    db.readDatabaseFile(path_db)

    if do_download:
        db.cloneReposTo(path_thirdparty_designs)

    if do_generate_unit_tests:
        generate_unit_tests(db, path_thirdparty_designs, path_unit_tests)

    if do_write_back:
        write_back_err_count(db, path_err_cnt_log, path_thirdparty_designs)

        db.writeDatabaseFile(path_db)

''' doit '''

import platform
import os

EXEC_EXT = ''
if platform.system() == 'Windows':
    EXEC_EXT = '.exe'

BUILD_DIR = 'build' + os.sep
if platform.system() == 'Windows':
    BUILD_DIR += 'windows' + os.sep
else:
    BUILD_DIR += 'linux' + os.sep

try:
    os.makedirs(BUILD_DIR)
except FileExistsError:
    pass


def task_lexer():
    ''' Build the lexer class '''

    global BUILD_DIR

    return {
        'file_dep' : ['QuarkLexer.cpp', 'QuarkLexer.h'],
        'targets' : ['%sQuarkLexer.o' % (BUILD_DIR)],
        'actions' : ['g++ -std=c++17 -g -c QuarkLexer.cpp -o %sQuarkLexer.o' % (BUILD_DIR)],
    }

def task_main():
    ''' Build the main function '''

    global EXEC_EXT, BUILD_DIR

    exec_file = '%smain_lexer%s' % (BUILD_DIR, EXEC_EXT)

    return {
        'file_dep' : ['main.cpp', '%sQuarkLexer.o' % (BUILD_DIR)],
        'targets' : [exec_file],
        'actions' : ['g++ -std=c++17 main.cpp %sQuarkLexer.o -g -o %s -lgmp -lgmpxx' % (BUILD_DIR, exec_file)],
    }


# def task_flex():
#     ''' Run flex '''

#     return {
#         'actions' : ['flex++ quark.l', 'g++ -o calc.exe lex.yy.c -lfl'],
#     }


# def task_test():
#     ''' Build the lexer??? '''

#     return {
#         'actions' : ['.\\lexer.exe']
#     }

# Just run doit in the folder

'''
# You can have a lot of fun with this

def task_data():
    """Step 1: generate some data for the subject"""
    for subject in subjects:
        yield {
            'name': subject,
            'file_dep': ['data.py'],
            'targets': ['%s-data.txt' % subject],
            'actions': ['python data.py %s' % subject],
        }


def task_filter():
    """Step 2: low-pass filter the data at 30 Hz to clean it"""
    for subject in subjects:
        yield {
            'name': subject,
            'file_dep': ['filter.py',
                         '%s-data.txt' % subject],
            'targets': ['%s-filtered_data.txt' % subject],
            'actions': ['python filter.py %s' % subject],
        }


'''
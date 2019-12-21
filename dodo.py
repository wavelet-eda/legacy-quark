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


def task_quark():
    ''' Build the quark class '''

    global BUILD_DIR

    return {
        'file_dep' : ['Quark.cpp', 'Quark.h'],
        'targets' : ['%sQuark.o' % (BUILD_DIR)],
        'actions' : ['g++ -std=c++17 -g -c Quark.cpp -o %sQuark.o' % (BUILD_DIR)],
    }

def task_quark_lexer():
    ''' Build the lexer class '''

    global BUILD_DIR

    return {
        'file_dep' : ['QuarkLexer.cpp', 'QuarkLexer.h'],
        'targets' : ['%sQuarkLexer.o' % (BUILD_DIR)],
        'actions' : ['g++ -std=c++17 -g -c QuarkLexer.cpp -o %sQuarkLexer.o' % (BUILD_DIR)],
    }

def task_quark_parser():
    ''' Build the parser class '''

    global BUILD_DIR

    return {
        'file_dep' : ['QuarkParser.cpp', 'QuarkParser.h'],
        'targets' : ['%sQuarkParser.o' % (BUILD_DIR)],
        'actions' : ['g++ -std=c++17 -g -c QuarkParser.cpp -o %sQuarkParser.o' % (BUILD_DIR)],
    }

def task_main():
    ''' Build the main function '''

    global EXEC_EXT, BUILD_DIR

    exec_file = '%smain_lexer%s' % (BUILD_DIR, EXEC_EXT)
    quark_o = '%sQuark.o' % (BUILD_DIR)
    quark_lexer_o = '%sQuarkLexer.o' % (BUILD_DIR)
    quark_parser_o = '%sQuarkParser.o' % (BUILD_DIR)

    return {
        'file_dep' : ['main.cpp', quark_o, quark_lexer_o, quark_parser_o],
        'targets' : [exec_file],
        'actions' : ['g++ -std=c++17 %s %s %s main.cpp -g -o %s -lgmp -lgmpxx' % (
            quark_o, quark_lexer_o, quark_parser_o, exec_file)],
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
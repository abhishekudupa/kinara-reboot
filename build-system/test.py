#!/usr/bin/python

import configtester, configuration, configure, find_builders

if __name__ == '__main__':
    find_builders.find_basic_builders()
    configuration.the_global_project_configuration['project_root'] = '/home/audupa/work-all/Penn/protsynth/git/kinara'
    configure.configure()
    print(configuration.the_global_project_configuration)

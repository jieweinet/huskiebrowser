# Copyright 2020 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import sys
import os

sys.path += [os.path.dirname(os.path.dirname(__file__))]

from style_variable_generator.css_generator import CSSStyleGenerator
from style_variable_generator.presubmit_support import RunGit


# TODO(calamity): extend this checker to find unused C++ variables
def FindInvalidCSSVariables(file_to_json_strings, git_runner=RunGit):
    style_generator = CSSStyleGenerator()
    css_prefixes = set()
    for f, json_string in file_to_json_strings.items():
      style_generator.AddJSONToModel(json_string, in_file=f)

      context = style_generator.in_file_to_context.get(f, {}).get('CSS')
      if (not context or 'prefix' not in context):
          raise KeyError('This tool only works on files with a CSS prefix.')

      css_prefixes.add('--' + context['prefix'] + '-')

    valid_names = style_generator.GetCSSVarNames()

    for css_prefix in css_prefixes:
      found_names_list = git_runner([
          'grep', '-ho',
          '\\%s[a-z0-9-]*' % css_prefix, '--', '*.css', '*.html', '*.js'
      ]).decode('utf-8').splitlines()
      found_names = set()
      for name in found_names_list:
          rgb_suffix = '-rgb'
          if name.endswith(rgb_suffix):
              name = name[:-len(rgb_suffix)]
          found_names.add(name)
    return {
        'unspecified': found_names.difference(valid_names),
        'unused': valid_names.difference(found_names),
        'css_prefix': css_prefix,
    }


def main():
    parser = argparse.ArgumentParser(
        description='''Finds CSS variables in the codebase that are prefixed
        with |input_files|' CSS prefix but aren't specified in |input_files|.'''
    )
    parser.add_argument('targets', nargs='+', help='source json5 color files', )
    args = parser.parse_args()

    input_files = args.targets

    file_to_json_strings = {}
    for input_file in input_files:
        with open(input_file, 'r') as f:
            file_to_json_strings[input_file] = f.read()

    result = FindInvalidCSSVariables(file_to_json_strings)

    print('Has prefix %s but not in %s:' % (result['css_prefix'], input_file))
    for name in sorted(result['unspecified']):
        print(name)

    print('\nGenerated by %s but not used in codebase:' % input_file)
    for name in sorted(result['unused']):
        print(name)

    return 0


if __name__ == '__main__':
    sys.exit(main())

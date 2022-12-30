#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import icdump
import argparse
from pathlib import Path
from typing import Optional

def process(filepath: str, skip_protocols: bool = False,
            output_path: Optional[str] = None) -> int:
    target = Path(filepath)
    if not target.is_file():
        print(f"'{target}' is not a valid file", file=sys.stderr)
        return 1

    metadata = icdump.objc.parse(target.as_posix())
    if metadata is None:
        print(f"Can't parse ObjC metadata in {target}'", file=sys.stderr)
        return 1

    if skip_protocols:
        output = ""
        for cls in metadata.classes:
            output += cls.to_decl()
    else:
        output = metadata.to_decl()
    print(output)

    if output_path is not None:
        out = Path(output_path)
        if out.is_dir():
            out /= f"{target.name}_objc.h"
            out.write_text(output)
            print(f"Saved in {out}")
        else:
            print(f"Saved in {out}")
            out.write_text(output)
    return 0


def main() -> int:
    version = f"iCDump version: {icdump.__version__}"
    description = """
    iCDump, a modern ObjC class dump based on LIEF and LLVM
    """
    parser = argparse.ArgumentParser(epilog=version, description=description)
    parser.add_argument('-o', '--output',
                        help='Output file',
                        default=None)
    parser.add_argument('--skip-protocols',
                        help='Skip ObjC protocols definition',
                        action='store_true')
    parser.add_argument("file", help='Mach-O file')

    logger_group = parser.add_argument_group('Logger')
    verbosity = logger_group.add_mutually_exclusive_group()

    verbosity.add_argument('--debug',
            dest='main_verbosity',
            action='store_const',
            const=icdump.LOG_LEVEL.DEBUG)

    verbosity.add_argument('--trace',
            dest='main_verbosity',
            action='store_const',
            const=icdump.LOG_LEVEL.TRACE)

    verbosity.add_argument('--info',
            dest='main_verbosity',
            action='store_const',
            const=icdump.LOG_LEVEL.INFO)

    verbosity.add_argument('--warn',
            dest='main_verbosity',
            action='store_const',
            const=icdump.LOG_LEVEL.WARN)

    verbosity.add_argument('--err',
            dest='main_verbosity',
            action='store_const',
            const=icdump.LOG_LEVEL.ERR)

    verbosity.add_argument('--critical',
            dest='main_verbosity',
            action='store_const',
            const=icdump.LOG_LEVEL.CRITICAL)

    parser.set_defaults(main_verbosity=icdump.LOG_LEVEL.INFO)
    args = parser.parse_args()

    icdump.set_log_level(args.main_verbosity)

    return process(args.file, args.skip_protocols, args.output)

if __name__ == "__main__":
    sys.exit(main())

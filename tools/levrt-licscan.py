#!/usr/bin/env python3
"""levrt-licscan: scan a source tree for per-file license/copyright evidence.

usage: python -I levrt-licscan.py <root> [--csv out.csv] [--summary]

For every text file the header (first 60 lines) is searched for SPDX ids and
well-known license phrases. Each file gets a verdict:
  ok        MIT / BSD-2/3 / CC0 / LGPL-2.1+ evidence, nothing contradicting
  repo      no file-level license text; the root LICENSE file (checked for MIT/BSD/LGPL)
            governs. Recorded as "repo-level" in PROVENANCE, still listed per file
  flag      evidence of EPL/GPL-only/Apache/proprietary, or more than one license family
Directory LICENSE/NOTICE files are listed separately. Exit code 0 always:
the output is evidence for PROVENANCE.md, a human reviews flags.
"""
import csv, os, re, sys

EXT = {".h", ".hpp", ".hxx", ".c", ".cc", ".cpp", ".cxx", ".idl", ".inl", ".py", ".cmake",
       ".txt", ".md", ".xaml", ".cs", ".rs", ".js", ".ps1", ".cmd", ".props", ".targets",
       ".vcxproj", ".sln", ".json", ".yml", ".yaml", ".rc", ".def", ".asm", ".tt", ".g4", ""}
PATTERNS = [
    ("MIT", r"permission is hereby granted, free of charge|\bMIT License\b|SPDX-License-Identifier:\s*MIT|Licensed under the MIT"),
    ("BSD", r"Redistribution and use in source and binary forms|SPDX-License-Identifier:\s*BSD"),
    ("CC0", r"CC0 1\.0|SPDX-License-Identifier:\s*CC0"),
    ("LGPL-2.1+", r"GNU Lesser General Public|SPDX-License-Identifier:\s*LGPL"),
    ("GPL", r"GNU General Public License|SPDX-License-Identifier:\s*GPL"),
    ("Apache-2.0", r"Apache License|SPDX-License-Identifier:\s*Apache"),
    ("EPL", r"Eclipse Public License|SPDX-License-Identifier:\s*EPL"),
    ("BSL-1.0", r"Boost Software License|SPDX-License-Identifier:\s*BSL"),
    ("proprietary", r"Microsoft Confidential|proprietary and confidential|proprietary"),
]
OKSET = {"MIT", "BSD", "CC0", "LGPL-2.1+", "BSL-1.0"}
BADSET = {"EPL", "Apache-2.0", "GPL", "proprietary"}
COPY = re.compile(r"copyright\s*(\(c\)|©)?\s*[^\n]{0,100}", re.I)

def scan(root):
    rows, notices = [], []
    repo_ok = False
    for name in ("LICENSE", "LICENSE.md", "LICENSE.txt", "COPYING"):
        lp = os.path.join(root, name)
        if os.path.isfile(lp):
            t = open(lp, encoding="utf-8", errors="replace").read(4000)
            repo_ok = bool(re.search(r"permission is hereby granted|Redistribution and use|GNU Lesser", t, re.I))
            break
    for dp, dn, fn in os.walk(root):
        dn[:] = [d for d in dn if d != ".git"]
        for f in fn:
            p = os.path.join(dp, f)
            rel = os.path.relpath(p, root).replace("\\", "/")
            if re.match(r"(license|notice|copying|third.?party)", f, re.I):
                notices.append(rel)
            if os.path.splitext(f)[1].lower() not in EXT:
                continue
            try:
                with open(p, encoding="utf-8", errors="replace") as fh:
                    head = "".join(next(fh, "") for _ in range(60))
            except OSError:
                continue
            found = [n for n, rx in ((x[0], x[1]) for x in PATTERNS) if re.search(rx, head, re.I)]
            m = COPY.search(head)
            copy = m.group(0).strip()[:80] if m else ""
            fam = set(found)
            if fam & BADSET:
                verdict = "flag"
            elif fam & OKSET and len(fam & OKSET) == 1:
                verdict = "ok"
            elif not fam:
                verdict = "repo" if repo_ok else "flag"
            else:
                verdict = "flag"
            rows.append((rel, ",".join(sorted(fam)) or "none", copy, verdict))
    return rows, notices

def main():
    root = sys.argv[1]
    rows, notices = scan(root)
    if "--csv" in sys.argv:
        out = sys.argv[sys.argv.index("--csv") + 1]
        with open(out, "w", newline="", encoding="utf-8") as fh:
            w = csv.writer(fh)
            w.writerow(["file", "license_evidence", "copyright_line", "verdict"])
            w.writerows(rows)
    by = {}
    for r in rows:
        by.setdefault((r[1], r[3]), 0)
        by[(r[1], r[3])] += 1
    print("files scanned:", len(rows))
    for k, v in sorted(by.items(), key=lambda x: -x[1]):
        print(f"{v:6d}  {k[1]:5s}  {k[0]}")
    print("license/notice files:", *notices[:40], sep="\n  ")

if __name__ == "__main__":
    main()

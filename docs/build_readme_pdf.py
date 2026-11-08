#!/usr/bin/env python3
"""Build README.pdf from README.md using only the Python standard library.

    python3 docs/build_readme_pdf.py            # rewrite README.pdf
    python3 docs/build_readme_pdf.py --check    # exit 1 if README.pdf is stale

The output is deterministic: no timestamps, no random ids, so the same
README.md always yields byte-identical bytes. It supports exactly the Markdown
subset README.md uses: #/##/### headings, paragraphs, "- " lists (with indented
continuation lines), fenced code blocks, pipe tables and `inline code`. Text is
set in the standard PDF fonts (Helvetica, Helvetica-Bold, Courier), which every
viewer provides, so nothing is embedded.
"""

from __future__ import annotations

import sys
from dataclasses import dataclass
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "README.md"
OUTPUT = ROOT / "README.pdf"

PAGE_W, PAGE_H = 612, 792
MARGIN_X = 54
MARGIN_TOP = 60
MARGIN_BOTTOM = 60
TEXT_W = PAGE_W - 2 * MARGIN_X

# Glyph widths (1/1000 em) of Helvetica for ASCII 32..126; Courier is 600 everywhere.
HELVETICA = [
    278, 278, 355, 556, 556, 889, 667, 191, 333, 333, 389, 584, 278, 333, 278, 278,
    556, 556, 556, 556, 556, 556, 556, 556, 556, 556, 278, 278, 584, 584, 584, 556,
    1015, 667, 667, 722, 722, 667, 611, 778, 722, 278, 500, 667, 556, 833, 722, 778,
    667, 778, 722, 667, 611, 722, 667, 944, 667, 667, 611, 278, 278, 278, 469, 556,
    333, 556, 556, 500, 556, 556, 278, 556, 556, 222, 222, 500, 222, 833, 556, 556,
    556, 556, 333, 500, 278, 556, 500, 722, 500, 500, 500, 334, 260, 334, 584,
]
BOLD_FACTOR = 1.08  # Helvetica-Bold is slightly wider; its widths are estimated from Helvetica.

FONT_RES = {"R": "F1", "B": "F2", "C": "F3"}
FONT_NAMES = {"F1": "Helvetica", "F2": "Helvetica-Bold", "F3": "Courier"}

BODY = 10.0
BODY_LEADING = 13.5
CODE = 8.0
CODE_LEADING = 10.5
CODE_PAD = 6.0
TABLE = 8.5
TABLE_LEADING = 11.0
CELL_PAD = 4.0
HEADING_SIZE = {1: 21.0, 2: 14.5, 3: 11.5}
HEADING_BEFORE = {1: 0.0, 2: 16.0, 3: 11.0}
HEADING_AFTER = {1: 8.0, 2: 6.0, 3: 4.0}
BULLET_INDENT = 14.0
PARAGRAPH_GAP = 6.0
BULLET = "\\225"  # WinAnsi bullet


@dataclass(frozen=True)
class Word:
    text: str
    font: str  # "R", "B" or "C"
    glued: bool = False  # no space before it: it continues the previous word


def width(text: str, font: str, size: float) -> float:
    if font == "C":
        return 0.6 * size * len(text)
    total = sum(HELVETICA[ord(c) - 32] for c in text) * size / 1000
    return total * BOLD_FACTOR if font == "B" else total


def check_ascii(text: str) -> None:
    for number, line in enumerate(text.splitlines(), start=1):
        for char in line:
            if not 32 <= ord(char) <= 126:
                raise ValueError(f"README.md:{number}: non-printable-ASCII character {char!r}")


def inline(text: str, base: str = "R") -> list[Word]:
    """Split text into words, using Courier inside `backticks`.

    A word that touches a code span without whitespace (like the comma in
    "`a`, b") is marked glued so it is never separated from it on wrap.
    """
    words: list[Word] = []
    previous_part = " "  # pretend whitespace before the text so the first word is not glued
    for index, part in enumerate(text.split("`")):
        font = "C" if index % 2 else base
        for number, piece in enumerate(part.split()):
            touches_previous = number == 0 and not part[:1].isspace() and not previous_part[-1:].isspace()
            words.append(Word(piece, font, glued=touches_previous))
        previous_part = part or previous_part
    return words


def gap(previous: Word | None, word: Word, size: float) -> float:
    if previous is None or word.glued:
        return 0.0
    return width(" ", previous.font, size)


def units(words: list[Word]) -> list[list[Word]]:
    """Group words into unbreakable units (a word plus the words glued to it)."""
    grouped: list[list[Word]] = []
    for word in words:
        if word.glued and grouped:
            grouped[-1].append(word)
        else:
            grouped.append([word])
    return grouped


def unit_width(unit: list[Word], size: float) -> float:
    return sum(width(w.text, w.font, size) for w in unit)


def wrap(words: list[Word], size: float, max_w: float) -> list[list[Word]]:
    lines: list[list[Word]] = []
    current: list[Word] = []
    used = 0.0
    for unit in units(words):
        unit_w = unit_width(unit, size)
        lead = gap(current[-1] if current else None, Word("", unit[0].font), size)
        if current and used + lead + unit_w > max_w:
            lines.append(current)
            current, used, lead = [], 0.0, 0.0
        used += lead + unit_w
        current.extend(unit)
    if current:
        lines.append(current)
    return lines or [[]]


def escape(text: str) -> str:
    return text.replace("\\", "\\\\").replace("(", "\\(").replace(")", "\\)")


class Layout:
    def __init__(self) -> None:
        self.pages: list[list[str]] = [[]]
        self.y = PAGE_H - MARGIN_TOP

    # -- low-level drawing ----------------------------------------------------
    def emit(self, op: str) -> None:
        self.pages[-1].append(op)

    def new_page(self) -> None:
        self.pages.append([])
        self.y = PAGE_H - MARGIN_TOP

    def ensure(self, height: float) -> bool:
        """Start a new page unless `height` fits; True if a page break happened."""
        if self.y - height < MARGIN_BOTTOM and self.y < PAGE_H - MARGIN_TOP:
            self.new_page()
            return True
        return False

    def text_line(self, words: list[Word], x: float, baseline: float, size: float) -> None:
        """Draw one line; consecutive words of one font become one text run."""
        cursor = x
        run = ""
        run_font = ""
        run_x = x
        previous: Word | None = None

        def flush() -> None:
            if run:
                self.emit(f"BT /{FONT_RES[run_font]} {size:g} Tf {run_x:.2f} {baseline:.2f} Td ({escape(run)}) Tj ET")

        for word in words:
            font = word.font
            lead = gap(previous, word, size)
            if run and font == run_font:
                run += (" " if lead else "") + word.text
            else:
                flush()
                run, run_font, run_x = word.text, font, cursor + lead
            cursor += lead + width(word.text, font, size)
            previous = word
        flush()

    def rect(self, x: float, y: float, w: float, h: float, gray: float) -> None:
        self.emit(f"{gray:g} g {x:.2f} {y:.2f} {w:.2f} {h:.2f} re f 0 g")

    def hline(self, y: float, x0: float, x1: float, gray: float = 0.6) -> None:
        self.emit(f"{gray:g} G 0.5 w {x0:.2f} {y:.2f} m {x1:.2f} {y:.2f} l S 0 G")

    # -- blocks ---------------------------------------------------------------
    def heading(self, level: int, text: str) -> None:
        size = HEADING_SIZE[level]
        if self.y < PAGE_H - MARGIN_TOP:
            self.y -= HEADING_BEFORE[level]
        self.ensure(size * 1.4 + HEADING_AFTER[level] + 4 * BODY_LEADING)
        lines = wrap(inline(text, "B"), size, TEXT_W)
        for line in lines:
            self.y -= size * 1.2
            self.text_line(line, MARGIN_X, self.y, size)
        if level == 1:
            self.y -= 4
            self.hline(self.y, MARGIN_X, MARGIN_X + TEXT_W, 0.3)
        self.y -= HEADING_AFTER[level]

    def paragraph(self, text: str, indent: float = 0.0, bullet: bool = False) -> None:
        lines = wrap(inline(text), BODY, TEXT_W - indent)
        for number, line in enumerate(lines):
            self.ensure(BODY_LEADING)
            self.y -= BODY_LEADING
            if bullet and number == 0:
                self.emit(f"BT /F1 {BODY:g} Tf {MARGIN_X + indent - BULLET_INDENT + 3:.2f} {self.y:.2f} Td ({BULLET}) Tj ET")
            self.text_line(line, MARGIN_X + indent, self.y, BODY)
        self.y -= PARAGRAPH_GAP if not bullet else 2.5

    def code(self, lines: list[str]) -> None:
        box_w = TEXT_W
        capacity = int((box_w - 2 * CODE_PAD) / (0.6 * CODE))
        rows: list[str] = []
        for line in lines:
            line = line.expandtabs(4)
            while len(line) > capacity:
                rows.append(line[:capacity])
                line = "   " + line[capacity:]
            rows.append(line)
        self.y -= 2
        for index, row in enumerate(rows):
            self.ensure(CODE_LEADING + (2 * CODE_PAD if index == 0 else 0))
            top_pad = CODE_PAD if index == 0 else 0
            bottom_pad = CODE_PAD if index == len(rows) - 1 else 0
            self.y -= CODE_LEADING + top_pad
            self.rect(MARGIN_X, self.y - 3 - bottom_pad, box_w, CODE_LEADING + top_pad + bottom_pad, 0.94)
            if row:
                self.emit(f"BT /F3 {CODE:g} Tf {MARGIN_X + CODE_PAD:.2f} {self.y:.2f} Td ({escape(row)}) Tj ET")
            self.y -= bottom_pad
        self.y -= PARAGRAPH_GAP + 2

    def table(self, header: list[str], body: list[list[str]]) -> None:
        rows = [[inline(cell) for cell in row] for row in body]
        head = [inline(cell, "B") for cell in header]
        columns = len(head)

        def natural(cells: list[Word]) -> float:
            return sum(gap(cells[i - 1] if i else None, w, TABLE) + width(w.text, w.font, TABLE) for i, w in enumerate(cells))

        natural_w = [max(natural(r[i]) for r in [head] + rows) + 2 * CELL_PAD for i in range(columns)]
        min_w = [max((unit_width(u, TABLE) for r in [head] + rows for u in units(r[i])), default=0) + 2 * CELL_PAD for i in range(columns)]
        if sum(natural_w) <= TEXT_W:
            col_w = natural_w
        else:
            extra = TEXT_W - sum(min_w)
            grow = [n - m for n, m in zip(natural_w, min_w)]
            col_w = [m + extra * g / sum(grow) for m, g in zip(min_w, grow)]

        def draw(cells: list[list[Word]], header_row: bool) -> None:
            wrapped = [wrap(cell, TABLE, col_w[i] - 2 * CELL_PAD) for i, cell in enumerate(cells)]
            height = max(len(lines) for lines in wrapped) * TABLE_LEADING + 2 * 2
            if self.ensure(height + TABLE_LEADING) and not header_row:
                draw(head, True)
            top = self.y
            if header_row:
                self.rect(MARGIN_X, top - height, sum(col_w), height, 0.88)
            x = MARGIN_X
            for i, lines in enumerate(wrapped):
                for number, line in enumerate(lines):
                    baseline = top - 2 - (number + 1) * TABLE_LEADING + 3
                    self.text_line(line, x + CELL_PAD, baseline, TABLE)
                x += col_w[i]
            self.y = top - height
            self.hline(self.y, MARGIN_X, MARGIN_X + sum(col_w))

        self.y -= 2
        self.hline(self.y, MARGIN_X, MARGIN_X + sum(col_w))
        draw(head, True)
        for row in rows:
            draw(row, False)
        self.y -= PARAGRAPH_GAP + 2


def split_row(line: str) -> list[str]:
    placeholder = "\x00"
    cells = line.strip().strip("|").replace("\\|", placeholder).split("|")
    return [cell.strip().replace(placeholder, "|") for cell in cells]


def render(markdown: str) -> Layout:
    check_ascii(markdown)
    layout = Layout()
    lines = markdown.splitlines()
    index = 0
    while index < len(lines):
        line = lines[index]
        if line.startswith("```"):
            block: list[str] = []
            index += 1
            while not lines[index].startswith("```"):
                block.append(lines[index])
                index += 1
            layout.code(block)
            index += 1
        elif line.startswith("#"):
            level = len(line) - len(line.lstrip("#"))
            layout.heading(level, line[level:].strip())
            index += 1
        elif line.startswith("|"):
            header = split_row(line)
            index += 2  # skip the |---| separator row
            body: list[list[str]] = []
            while index < len(lines) and lines[index].startswith("|"):
                body.append(split_row(lines[index]))
                index += 1
            layout.table(header, body)
        elif line.startswith("- "):
            item = [line[2:].strip()]
            index += 1
            while index < len(lines) and lines[index].startswith("  ") and lines[index].strip():
                item.append(lines[index].strip())
                index += 1
            layout.paragraph(" ".join(item), indent=BULLET_INDENT, bullet=True)
        elif not line.strip():
            index += 1
        else:
            para = [line.strip()]
            index += 1
            while index < len(lines) and lines[index].strip() and not lines[index].startswith(("#", "```", "|", "- ")):
                para.append(lines[index].strip())
                index += 1
            layout.paragraph(" ".join(para))
    return layout


def build_pdf(layout: Layout) -> bytes:
    pages = layout.pages
    total = len(pages)
    objects: list[bytes] = [b"", b""]  # 1 catalog, 2 page tree (filled below)
    font_ids: dict[str, int] = {}
    for res, name in FONT_NAMES.items():
        objects.append(f"<< /Type /Font /Subtype /Type1 /BaseFont /{name} /Encoding /WinAnsiEncoding >>".encode("ascii"))
        font_ids[res] = len(objects)
    font_dict = " ".join(f"/{res} {font_ids[res]} 0 R" for res in FONT_NAMES)

    page_ids: list[int] = []
    for number, ops in enumerate(pages, start=1):
        footer = f"Rastera README - page {number} of {total}"
        foot_x = (PAGE_W - width(footer, "R", 8)) / 2
        ops = ops + [f"BT /F1 8 Tf {foot_x:.2f} 30 Td ({escape(footer)}) Tj ET"]
        stream = ("\n".join(ops) + "\n").encode("ascii")
        objects.append(b"<< /Length " + str(len(stream)).encode("ascii") + b" >>\nstream\n" + stream + b"endstream")
        content_id = len(objects)
        objects.append(
            f"<< /Type /Page /Parent 2 0 R /MediaBox [0 0 {PAGE_W} {PAGE_H}] "
            f"/Resources << /Font << {font_dict} >> >> /Contents {content_id} 0 R >>".encode("ascii")
        )
        page_ids.append(len(objects))
    objects.append(b"<< /Title (Rastera README) /Producer (docs/build_readme_pdf.py) >>")
    info_id = len(objects)
    objects[0] = b"<< /Type /Catalog /Pages 2 0 R >>"
    kids = " ".join(f"{pid} 0 R" for pid in page_ids)
    objects[1] = f"<< /Type /Pages /Kids [{kids}] /Count {total} >>".encode("ascii")

    out = bytearray(b"%PDF-1.4\n%\xe2\xe3\xcf\xd3\n")
    offsets: list[int] = []
    for number, body in enumerate(objects, start=1):
        offsets.append(len(out))
        out += f"{number} 0 obj\n".encode("ascii") + body + b"\nendobj\n"
    xref = len(out)
    out += f"xref\n0 {len(objects) + 1}\n0000000000 65535 f \n".encode("ascii")
    out += "".join(f"{offset:010d} 00000 n \n" for offset in offsets).encode("ascii")
    out += f"trailer\n<< /Size {len(objects) + 1} /Root 1 0 R /Info {info_id} 0 R >>\nstartxref\n{xref}\n%%EOF\n".encode("ascii")
    return bytes(out)


def main(argv: list[str]) -> int:
    if len(argv) > 1 and argv[1:] != ["--check"]:
        print("usage: build_readme_pdf.py [--check]", file=sys.stderr)
        return 2
    layout = render(SOURCE.read_text(encoding="ascii"))
    data = build_pdf(layout)
    if argv[1:] == ["--check"]:
        if not OUTPUT.exists() or OUTPUT.read_bytes() != data:
            print(f"{OUTPUT.name} is out of date: run python3 docs/build_readme_pdf.py", file=sys.stderr)
            return 1
        print(f"{OUTPUT.name} is up to date")
        return 0
    OUTPUT.write_bytes(data)
    print(f"wrote {OUTPUT.name} ({len(layout.pages)} pages, {len(data)} bytes)")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))

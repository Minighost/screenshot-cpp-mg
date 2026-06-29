from pathlib import Path

CUR_DIR = Path(__file__).parent
TARGET_DIR = "svgs"

for svg in (CUR_DIR / TARGET_DIR).iterdir():
    print(f"<file>{TARGET_DIR}/{svg.name}</file>")

print

for svg in (CUR_DIR / TARGET_DIR).iterdir():
    print(f"{TARGET_DIR}/{svg.name}")

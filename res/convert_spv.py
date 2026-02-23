import os
import subprocess
import sys

GLSLC = "glslc"

# valid Vulkan shader extensions
VALID_EXTENSIONS = {
    ".vert",
    ".frag",
}


def is_vk_shader(filename: str) -> bool:
    name, ext = os.path.splitext(filename)

    return (
        name.endswith("_vk") and
        ext in VALID_EXTENSIONS
    )


def compile_shader(filepath: str):
    output_path = filepath + ".spv"

    print(f"Compiling: {filepath}")

    result = subprocess.run(
        [GLSLC, filepath, "-o", output_path],
        capture_output=True,
        text=True
    )

    if result.returncode != 0:
        print(f"Failed to compile {filepath}")
        print(result.stderr)
    # else:
    #     print(f"Generated {output_path}")
    
    print("")


def main(root_dir: str):
    for root, dirs, files in os.walk(root_dir):
        for file in files:
            if is_vk_shader(file):
                full_path = os.path.join(root, file)
                compile_shader(full_path)


if __name__ == "__main__":
    if len(sys.argv) > 1:
        directory = sys.argv[1]
    else:
        directory = "."

    main(directory)

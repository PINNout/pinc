import os
import platform
import re
import subprocess
import tempfile

import lit.formats
import lit.util

from lit.llvm import llvm_config
from lit.llvm.subst import ToolSubst
from lit.llvm.subst import FindTool

config.name = "PINC"

config.test_format = lit.formats.ShTest()

config.suffixes = [".mlir"]

config.test_source_root = os.path.dirname(__file__)

config.test_exec_root = os.path.join(config.pinc_obj_root, "test")

config.substitutions.append(("%PATH%", config.environment["PATH"]))
config.substitutions.append(("%shlibtext", config.llvm_shlib_ext))

llvm_config.with_system_environment(["HOME", "INCLUDE", "TMP", "TEMP"])

llvm_config.use_default_substitutions()

config.excludes = ["CMakeLists.txt", "Temp"]

config.test_exec_root = os.path.join(config.pinc_obj_root, "test")
config.pinc_tools_dir = os.path.join(config.pinc_obj_root, "bin")
#config.pinc_libs_dir = os.path.join(config.pinc_obj_root, "lib")

#config.substitutions.append(("%pinc_libs", config.pinc_libs_dir))

llvm_config.with_environment("PATH", config.llvm_tools_dir, append_path=True)

tool_dirs = [config.pinc_tools_dir, config.llvm_tools_dir]
tools = [
    "mlir-opt",
    "pinc-opt",
]

llvm_config.add_tool_substitutions(tools, tool_dirs)

python_path = [os.path.join(config.mlir_obj_dir, "python_packages", "pinc")]
if "PYTHONPATH" in os.environ:
    python_path += [os.environ["PYTHONPATH"]]

llvm_config.with_environment("PYTHONPATH", python_path, append_path=True)

# Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved


# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
# import os
# import sys
# sys.path.insert(0, os.path.abspath('.'))


# -- Project information -----------------------------------------------------

project = "embers"
copyright = "2020 Advanced Micro Devices, Inc. All rights reserved"
author = "AMD"


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = ["breathe", "exhale"]

breathe_projects = {"embers": "./_doxygen/xml"}

breathe_default_project = "embers"

exhale_args = {
    "containmentFolder": "./api",
    "rootFileName": "embers_root.rst",
    "doxygenStripFromPath": "..",
    "rootFileTitle": "Embers API",
    "createTreeView": True,
    "exhaleExecutesDoxygen": True,
    "exhaleDoxygenStdin": "INPUT = ../include/embers",
}

primary_domain = "cpp"
highlight_language = "cpp"
cpp_id_attributes = ["__global__", "__host__", "__device__"]

# Add any paths that contain templates here, relative to this directory.
templates_path = ["_templates"]

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = "sphinx_rtd_theme"

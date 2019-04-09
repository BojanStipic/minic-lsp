# minic-lsp

[Language Server](https://langserver.org/) implementation for the miniC programming language.

## Features

**TODO…**

## What is Language Server Protocol?

The Language Server Protocol is used between a tool (the client)
and a language smartness provider (the server)
to integrate features like auto complete, go to definition,
find all references and alike into the tool.

## What is miniC?

miniC is a programming language created for educational purposes.
It is a strict subset of the C programming language.

miniC is used at the University of Novi Sad, Faculty of Technical Sciences, to teach the Compilers course.

## Prerequisites

Runtime dependencies:

* cJSON

Compile time dependencies:

* flex
* bison
* C standard compiler
* cJSON
* pkgconf
* make **or** meson build system

## Installation

Compile and install using Meson build system:
```bash
meson build --prefix=<installation/path>
ninja -C build install
```

Compile using GNU make utility (*not recommended*):
```bash
make
```

## Clients

* [Plugin](https://github.com/BojanStipic/minic-lsp-ale) for [Vim](https://www.vim.org/)
text editor with [ALE](https://github.com/w0rp/ale).
* [Plugin](https://github.com/BojanStipic/minic-lsp-atom) for [Atom](https://atom.io/)
text editor.
* … and any other client listed [here](https://langserver.org/), if you configure it to use minic-lsp.

## License

This work is based on the original micko compiler of the miniC programming language.

	Copyright (C) 2019 Bojan Stipic
	Copyright (C) 20XY-2018 Zorica Suvajdzin Rakic, Zarko Zivanov

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

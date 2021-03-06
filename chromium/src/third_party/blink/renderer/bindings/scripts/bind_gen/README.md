# Blink-V8 bindings generator (bind_gen package)

[TOC]

## What's bind_gen?

Python package
[`bind_gen`](https://source.chromium.org/chromium/chromium/src/+/main:third_party/blink/renderer/bindings/scripts/bind_gen/)
is the core part of Blink-V8 bindings code generator.
[`generate_bindings.py`](https://source.chromium.org/chromium/chromium/src/+/main:third_party/blink/renderer/bindings/scripts/generate_bindings.py)
is the driver script, which takes a Web IDL database (`web_idl_database.pickle`
generated by "web_idl_database" GN target) as an input and produces a set of
C++ source files of Blink-V8 bindings (v8_\*.h, v8_\*.cc).

## Design and code structure

The bindings code generator is implemented as a tree builder of `CodeNode`
which is a fundamental building block.  The following sub sections describe
what `CodeNode` is and how the code generator builds a tree of `CodeNode`.

### CodeNode

The code generator produces C++ source files (text files) but the content of
each file is not represented as a single giant string nor a list of strings.
The content of each file is represented as a CodeNode tree.

`CodeNode` (defined in
[code_node.py](https://source.chromium.org/chromium/chromium/src/+/main:third_party/blink/renderer/bindings/scripts/bind_gen/code_node.py))
is a fundamental building block that represents a text fragment in the tree
structure.  A text file is represented as a tree of CodeNodes, each of which
represents a corresponding text fragment.  The code generator is the CodeNode
tree builder.

Here is a simple example to build a CodeNode tree.
```python
# SequenceNode and TextNode are subclasses of CodeNode.

def prologue():
  return SequenceNode([
    TextNode("// Prologue"),
    TextNode("SetUp();"),
  ])

def epilogue():
  return SequenceNode([
    TextNode("// Epilogue"),
    TextNode("CleanUp();"),
  ])

def main():
  root_node = SequenceNode([
    prologue(),
    TextNode("LOG(INFO) << \"hello, world\";"),
    epilogue(),
  ])
```
The `root_node` above represents the following text.

```c++
// Prologue
SetUp();
LOG(INFO) << "hello, world";
// Epilogue
CleanUp();
```

The basic features of CodeNode are implemented in
[code_node.py](https://source.chromium.org/chromium/chromium/src/+/main:third_party/blink/renderer/bindings/scripts/bind_gen/code_node.py).
Just for convenience, CodeNode subclasses corresponding to C++ constructs are
provided in
[code_node_cxx.py](https://source.chromium.org/chromium/chromium/src/+/main:third_party/blink/renderer/bindings/scripts/bind_gen/code_node_cxx.py).

`CodeNode` has an object-oriented design and has internal states (not only the
parent / child nodes but also more states to support advanced features).

### CodeNode tree builders

The bindings code generator consists of multiple sub code generators.  For
example, `interface.py` is a sub code generator of Web IDL interface and
`enumeration.py` is a sub code generator of Web IDL enumeration.  Each Web IDL
definition has its own sub code generator.

This sub section describes how a sub code generator builds a CodeNode tree and
produces C++ source files by looking at
[`enumeration.py`](https://source.chromium.org/chromium/chromium/src/+/main:third_party/blink/renderer/bindings/scripts/bind_gen/enumeration.py)
as an example.  The example code snippet below is simplified for explanation.

```python
def generate_enumerations(task_queue):
    for enumeration in web_idl_database.enumerations:
        task_queue.post_task(generate_enumeration, enumeration.identifier)
```

`generate_enumerations` is the entry point to this sub code generator.  In
favor of parallel processing, `task_queue` is used.  `generate_enumeration`
(singular form) actually produces a pair of C++ source files (\*.h and \*.cc).

```python
def generate_enumeration(enumeration_identifier):
    # Filepaths
    header_path = path_manager.api_path(ext="h")
    source_path = path_manager.api_path(ext="cc")

    # Root nodes
    header_node = ListNode(tail="\n")
    source_node = ListNode(tail="\n")

    # ... fill the contents of `header_node` and `source_node` ...

    # Write down to the files.
    write_code_node_to_file(header_node, path_manager.gen_path_to(header_path))
    write_code_node_to_file(source_node, path_manager.gen_path_to(source_path))
```

The main task of `generate_enumeration` is to build the CodeNode trees and
write them down to files.  A key point here is to build two trees in parallel;
one for \*.h and the other for \*.cc.  We can add a function declaration to the
header file while adding the corresponding function definition to the source
file.  The following code snippet is an example to add constructors into the
header file and the source file.

```python
    # Namespaces
    header_blink_ns = CxxNamespaceNode(name_style.namespace("blink"))
    source_blink_ns = CxxNamespaceNode(name_style.namespace("blink"))
    # {header,source}_blink_ns are added to {header,source}_node (the root
    # nodes) respectively.

    # Class definition
    class_def = CxxClassDefNode(cg_context.class_name,
                                base_class_names=["bindings::EnumerationBase"],
                                final=True,
                                export=component_export(
                                    api_component, for_testing))

    ctor_decls, ctor_defs = make_constructors(cg_context)

    # Define the class in 'blink' namespace.
    header_blink_ns.body.append(class_def)

    # Add constructors to public: section of the class.
    class_def.public_section.append(ctor_decls)
    # Add constructors (function definitions) into 'blink' namespace in the
    # source file.
    source_blink_ns.body.append(ctor_defs)
```

In the above code snippet, `make_constructors` creates and returns a CodeNode
tree for the header file and another CodeNode tree for the source file.  For
most cases, functions named `make_xxx` creates and returns a pair of the
CodeNode trees.  These functions are subtree builders of the CodeNode trees.
They are implemented in a way of functional programming (unlike CodeNodes
themselves are implemented in a way of object-oriented programming).  These
subtree builders create a pair of new CodeNode trees at every function call
(returned code node instances are different per call, so their internal states
are separate), but the contents are 100% determined by the input arguments.
This property is very important when we use closures in advanced use cases.

So far, the typical code structure of the sub code generators is covered.
`enumeration.py` consists of several `make_xxx` functions (subtree builders) +
`generate_enumeration` (the top-level tree builder + file writer).

### Advanced: Auto-completion and declarative style

TODO(yukishiino): Write about `SymbolNode`, `SymbolDefinitionNode`,
`SequenceNode`, and `SymbolScopeNode`.

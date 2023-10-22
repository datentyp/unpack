# README

## Usage

`Usage: unpack [file1 file2 ...]`

Parses "TopicReaderExport" files and unpacks contained kafka message records in working directory.

When no file was provided as argument, or when file is —, unpack reads from standard input.

The TopicReaderExport file format starts with five lines containing metadata about the used
environment, topic, searchValue, timeFrom, timeTo parameters during the export followed by an arbitrary
number of lines representation exported kafka message records with CSV fields

* partition,offset,timestamp,key,value

Fields key and value _might_ be enclosed within `''`. If field `value` contains some JSON object it will be wrapped
in `''`. But if it contains just some number (001), the value of the field won't be wrapped with `''`.

By unpacking we get a single file for every line containing a proper kafka message record.

**Example**

```shell
$ cat file1.txt
```

```text
[1] environment: PROD
[2] topic      : comp.os.minix
[3] searchValue: 2023
[4] timeFrom   : 2023-08-05T00:00:00.0000000
[5] timeTo     : 2023-08-05T08:00:00.0000000
[6] 1,18890097,2023-08-05T00:00:17.0650000Z,"HAL7000",'{}'
...
```

Lines 1-5 contain some metadata. Beginning with line 6 all remaining lines contain a single comma separated record
following a structure like this:

| partition | offset   | timestamp                    | key       | value | 
|-----------|----------|------------------------------|-----------|-------|
| 1         | 18890097 | 2023-08-05T00:00:17.0650000Z | "HAL7000" | '{}'  |

Running `unpack` on this file will create a new file named `PROD/comp.os.minix/1/18890097.json5` that starts with a
line containing a JSON5 comment (//) with the metadata (environment, topic, partition, offset, timestamp, key) and the
content (aka. the value) of the last field (except the enclosing '') in the current directory.

```shell
$ unpack file.txt
$ cat PROD/comp.os.minix/1/18890097.json5
// environment=PROD, topic=comp.os.minix, partition=1, offset=18890097, timestamp=2023-08-05T00:00:17.0650000Z, key="HAL7000"
{}
```

## Install

To install the `unpack` binary you have to build the source (run `make` in the root directory of the project) and then
copy the generated `build/unpack` file to a directory that is part of your shell's search path for executables (see
`echo $PATH`).

You can set `INSTALL_DIR` in `Makefile` to build and install `unpack` with `make`:

```shell
$ make install
```

Note that `install` uses `${HOME}/bin` by default.

## KNOWN BUGS

It's good enough for my use case, but probably only because I'm aware of its limitations.

### No real ' (or JSON string) parsing

To extract key and value fields `unpack` might have to extract the content enclosed by `''` (single parenthesis).

If the content of key field itself contains a `'` other than except the real closing `'` `unpack` will get confused and
exit_on_failure to extract the correct values for fields key and value.

Example:

```text 
0,500,1991-07-04T10:00:50.0000000Z,'{"field1": "value1 \\',", "field2": "',value2"}','{"field1": "value1 ',", "field2": "',value2"}'
```

### JSON5 as hardcoded file extension

All unpacked files will be written to a `.json5` file because I usually have to deal with JSON representation
of AVRO encoded data. No real attempt to detect the "real" file type is being undertaken. JSON5 is used instead of JSON
to include a comment line containing some metadata about the record.

### No UTF-8 support - Limited to 8-bit characters

### No optimization - It copies stuff and is slow

Besides reading the input file, "tokenizing" and copying its parts it doesn't eat lots of memory, but for large input
files you need to be patient because it is super slow as it basically processes everything sequentially - bit by bit
(file by file, line by line, char by char).

### It's full of classic C bugs, errors, unnecessary unidiomatic code, off-by-one errors and bad style overall. Beginner stuff.

The tool was written in C because I haven't written any C code for about twenty years, and one Sunday morning I was
feeling a bit nostalgic for those old days. Actually, it was exciting and I didn't even notice that many problems
while writing unpacks (Java-esque like C) code. But later when I re-read some parts of the code to create this README
my missing C experience looked obvious. Argh. It was fun nonetheless!  

Did I mention that it lacks any tests?  Well, I lack the use case, time, and experience to improve it.

### It doesn't handle malformed data very well

## Tips

To format json files outputted by `unpack` use `jreformat`. It uses `jq` to format the file and is able to keep the
first line containing the metadata as JSON5 comment.

```shell
$ jreformat
```

## Development

Requirements

* Gcc Toolchain

### Build

```shell
make
```

### Debugging

Debug build, writes to `/tmp/unpack.debug.txt`

```shell
$ make clean debug install
```

Debugger

```shell
$ gdb -ex=run --args ./unpack file1
```

### Memory Leaks

#### Valgrind

```shell
$ valgrind --leak-check=full -s ./unpack file.txt
```

#### Leaks (Mac OS X)

```shell
$ export MallocStackLogging=1
$ leaks --atExit -- ./unpack file.txt
```

See:

* https://developers.redhat.com/blog/2021/04/23/valgrind-memcheck-different-ways-to-lose-your-memory#
* https://computerscience.chemeketa.edu/guides/valgrind/leaks/

## Other

* Besides other related tools to automate all kinds of stuff `workbench` (`kk`) contains an older (but feature enhanced)
  Kotlin based version of `unpack`.
* Use `deck` to download properly named TopicReaderExport files.
* TopicReaderExport files are not properly sorted (by timestamp). Use `treftsfc` to fix that.
* Use `jreformat` to format unpacked json files.
* Use `$ bat -l json <environment>/<topic>/<partition>/<offset>.json5` to view files.
* Use `kaka` (`kk`) if you need a real Kafka client.
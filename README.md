# covid
![Tests](https://github.com/arashbm/covid/workflows/Tests/badge.svg) ![Lint](https://github.com/arashbm/covid/workflows/Lint/badge.svg)

Implementation of model from [Arenas et al.][arenas] and revisions by Eva Kisdi
and others.

[arenas]: https://doi.org/10.1101/2020.03.21.20040022

## Getting started

Clone everything, including submodules:

```bash
$ git clone --recursive https://github.com/arashbm/covid.git
```

Compile the library and run the tests:

```bash
$ cd covid
$ make
```

Check out the tests at `/src/test/covid/` for some examples on how to use
everything.

After making modifications, make sure tests still succeed:

```bash
$ make
```

and that there are no code styling issue (this is a guideline and not a rule):

```bash
$ make lint
```

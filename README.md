# CEGARBoxCPP

## Versions
I will be working on CEGARBoxCPP continuously throughout the year, so the master branch will change. Because of this I will create branches for each version of the tool.
For the submission to TABLEAUX2023, check the TABLEAUX2023 branch.
For the submission to CSL2024, check the CSL2024 branch.

## Authorship
Robert McArthur and Cormac Kikkert, for contact email cormac.kikkert@anu.edu.au

## Programs
There are two programs in this repo.

`kaleidoscope`: CEGARBox++(K), handles K extended with single axioms OR global assumptions OR converse modalities (tense logic).

`lumen`: CEGARBox++(LTL), handles LTL and LTL-f

## Compile
A statically compiled file is already available in the repo - compiled on Ubuntu 20.04.3.

However, if you want to recreate our results, you will need to compile CEGARBoxCPP for your machine. To do this, follow the instructions below:

1. Remove the `-static` flag in the makefile
2. Run ``make`` to compile CEGARBox

Note that the following libraries will be installed:
1. [Minisat](https://github.com/agurfinkel/minisat), a SAT-solver. This is a fork, as the original is outdated and no longer works.
2. [antlr4](https://www.antlr.org/download.html) for LTL parsing. This is the C++ target
3. [ltl2snf](https://nalon.org/#software) for converting LTL formula to SNF.


## Input Formula
CEGARBox(K) accepts file input. Input is terminated by a newline and valid input formula are defined by the following grammar:
```
Index ::= Nat || -Nat
Atom ::= Alphanumeric String
Formula ::=
 Atom || $true || $false || ~Formula ||
 [Index] Formula || <Index> Formula || []Formula || <>Formula || 
 Formula | Formula || Formula & Formula || Formula => Formula || 
 Formula <=> Formula || (Formula) 
```

Here, negative numbers are used to define converse for tense logic. For example ``[-1]`` is the converse of ``[1]``.

CEGARBox(K) does not handle intohylo files! So files with BEGIN and END won't work. Please refer to the examples in `Examples`.

CEGARBox(LTL) uses the same input as ltl2snf:
```
PROPOSITIONAL SYMBOLS: an alfanumeric sequence starting with a letter: p, p1, p_1 (underscore should not be used at the beginning of a name)
CONSTANTS: true, false
NOT: -, ~, not
AND: &, and
OR: |, or
IMPLICATION: ->, =>, imp, imply, implies
ONLY IF: <-, <=
DOUBLE IMPLICATION: <->, <=>, iff
ALWAYS: [], always
EVENTUALLY: <>, sometime
UNTIL: U, until
UNLESS: W, unless
```

## Running CEGARBox 

``./main -f <input_file> [options]``

Options:

* Reflexivity: ``--reflexive`` or ``-t``
* Symmetry: ``--symmetric`` or ``-b``
* Transitivity: ``--transitive`` or ``-4``
* Seriality: ``--serial`` or ``-d``
* Euclidean: ``--euclidean`` or ``-e``
* Valid (whether input formula is valid): ``--valid`` or ``-a``
* Tense: ``--tense`` or `-n`
* Verbose: ``--verbose`` or `-v`

To use the local prover add `-l`
To use the global prover add `-g`
(Default is bespoke)

## Benchmarks

MQBF, 3CNF and LWB_K benchmarks can be downloaded from [here](http://www.cril.univ-artois.fr/~montmirail/mosaic/#)
ALC benchmarks can be downloaded from [here](https://web.archive.org/web/20190305011522/http://users.cecs.anu.edu.au/~rpg/BDDTab/)

## Examples
Some examples to see how to use the tool are in `Examples`.

Note: logics besides K, KD, KT, KB, K4, K5, and tense are untested, and may not work.
Also, we have not extensively tested multimodal logics either - the standard benchmarks are all monomodal

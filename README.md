## Simple Compiler v4

A teaching compiler that uses a subset of a C++-like input language. The language includes integer literals, arithmetic operators (+, -, *, !), variable declarations/assignments/increments, top-level functions with parameters and return statements, single-inheritance classes with public virtual methods, object instantiation, dynamic method dispatch via vtables (including overrides), do-while loops with less-than comparisons, and statement blocks, while generating correct output for LLVM IR, and x64 assembly assembly backends.

Version 1 available [here](https://github.com/meuer26/SimpleCompilerv1). Version 3 available [here](https://github.com/meuer26/SimpleCompilerv3_Student). The first three versions were written between 2023-2026 for teaching compiler concepts.

Version 4 was created in 2026 by taking the existing working Version 3 base code and re-organizing it with the help of Claude Opus 4.7 and 4.8 (Anthropic) at Dan O'Malley's direction to support a clean visitor pattern with AST, graph coloring and liveness pedagogy, as well as basic OOP. I dropped the ARM backend in version 4 to focus more effort on optimizations and advanced language constructs.



## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.




## Attribution

- Base code and test cases for versions 1-3 were written by Dan O'Malley between 2023-2026.
- Version 4 was created in 2026 by taking the existing working Version 3 base code and re-organizing it with the help of Claude Opus 4.7 and 4.8 (Anthropic) at Dan O'Malley's direction to support a clean visitor pattern with AST, graph coloring and liveness pedagogy, as well as basic OOP. I dropped the ARM backend in version 4 to focus most effort on optimizations and advanced language constructs.
- Progress bar in run_all_test_cases_for_errors.sh was modified from [Baeldung](https://www.baeldung.com/linux/command-line-progress-bar) and that is noted in that file.
- Parallel testing code in run_all_test_cases_for_errors.sh was written by Grok v4 (xAI) and that is noted in that file. 
/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "Container System", "index.html", [
    [ "System Overview", "index.html#overview", null ],
    [ "Key Features", "index.html#features", [
      [ "Value Type Reference", "index.html#value_type_table", null ]
    ] ],
    [ "Architecture Diagram", "index.html#architecture", null ],
    [ "Quick Start", "index.html#quickstart", null ],
    [ "Installation", "index.html#installation", [
      [ "CMake FetchContent (Recommended)", "index.html#install_fetchcontent", null ],
      [ "vcpkg", "index.html#install_vcpkg", null ],
      [ "Manual Build", "index.html#install_manual", null ]
    ] ],
    [ "Module Overview", "index.html#modules", [
      [ "Performance Benchmarks", "index.html#perf_benchmarks", null ]
    ] ],
    [ "Examples", "index.html#examples", null ],
    [ "Learning Resources", "index.html#learning", null ],
    [ "Related Systems", "index.html#related", null ],
    [ "Container System Examples", "md_examples_2README.html", [
      [ "Examples", "md_examples_2README.html#autotoc_md1", [
        [ "async_coroutine_example", "md_examples_2README.html#autotoc_md2", null ],
        [ "asio_integration_example", "md_examples_2README.html#autotoc_md3", null ],
        [ "messaging_integration_example", "md_examples_2README.html#autotoc_md4", null ]
      ] ],
      [ "Building Examples", "md_examples_2README.html#autotoc_md5", null ],
      [ "Running Examples", "md_examples_2README.html#autotoc_md6", null ],
      [ "Configuration Options", "md_examples_2README.html#autotoc_md7", [
        [ "Messaging Features", "md_examples_2README.html#autotoc_md8", null ],
        [ "Performance Metrics", "md_examples_2README.html#autotoc_md9", null ],
        [ "External Integration", "md_examples_2README.html#autotoc_md10", null ],
        [ "Combined Configuration", "md_examples_2README.html#autotoc_md11", null ]
      ] ],
      [ "Expected Output", "md_examples_2README.html#autotoc_md12", null ],
      [ "Integration with Messaging Systems", "md_examples_2README.html#autotoc_md13", null ]
    ] ],
    [ "Tutorial: Container Basics", "tutorial_containers.html", [
      [ "Introduction", "tutorial_containers.html#tut_cont_intro", null ],
      [ "Value Type Selection Guide", "tutorial_containers.html#tut_cont_value_types", [
        [ "Type Selection Examples", "tutorial_containers.html#tut_cont_type_examples", null ]
      ] ],
      [ "Container Builder", "tutorial_containers.html#tut_cont_builder", [
        [ "Factory-Based Construction", "tutorial_containers.html#tut_cont_builder_factory", null ]
      ] ],
      [ "Iteration Patterns", "tutorial_containers.html#tut_cont_iteration", [
        [ "Direct Lookup", "tutorial_containers.html#tut_cont_iter_lookup", null ],
        [ "Range-Based Iteration", "tutorial_containers.html#tut_cont_iter_range", null ],
        [ "Nested Container Traversal", "tutorial_containers.html#tut_cont_iter_nested", null ]
      ] ],
      [ "Next Steps", "tutorial_containers.html#tut_cont_next", null ]
    ] ],
    [ "Tutorial: Serialization and SIMD", "tutorial_serialization.html", [
      [ "Serialization Overview", "tutorial_serialization.html#tut_ser_overview", null ],
      [ "Binary Format", "tutorial_serialization.html#tut_ser_binary", [
        [ "Binary Round Trip", "tutorial_serialization.html#tut_ser_binary_example", null ],
        [ "Selecting a Different Format", "tutorial_serialization.html#tut_ser_format_select", null ]
      ] ],
      [ "SIMD Acceleration", "tutorial_serialization.html#tut_ser_simd", [
        [ "Letting SIMD Kick In", "tutorial_serialization.html#tut_ser_simd_example", null ]
      ] ],
      [ "Cross-Platform Compatibility", "tutorial_serialization.html#tut_ser_compat", [
        [ "Format Versioning", "tutorial_serialization.html#tut_ser_compat_versioning", null ]
      ] ],
      [ "Next Steps", "tutorial_serialization.html#tut_ser_next", null ]
    ] ],
    [ "Tutorial: Integration Patterns", "tutorial_integration.html", [
      [ "Where Container System Fits", "tutorial_integration.html#tut_int_overview", null ],
      [ "Integration with network_system", "tutorial_integration.html#tut_int_network", null ],
      [ "Integration with database_system", "tutorial_integration.html#tut_int_database", null ],
      [ "Messaging Integration", "tutorial_integration.html#tut_int_messaging", null ],
      [ "Best Practices", "tutorial_integration.html#tut_int_best_practices", null ],
      [ "Next Steps", "tutorial_integration.html#tut_int_next", null ]
    ] ],
    [ "Frequently Asked Questions", "faq.html", [
      [ "Which value type should I use for my data?", "faq.html#faq_value_type", null ],
      [ "What are the SIMD requirements and what happens on fallback?", "faq.html#faq_simd", null ],
      [ "What are the thread safety guarantees?", "faq.html#faq_thread_safety", null ],
      [ "What is the maximum container size?", "faq.html#faq_max_size", null ],
      [ "Can I nest containers?", "faq.html#faq_nested", null ],
      [ "What kind of performance can I expect?", "faq.html#faq_performance", null ],
      [ "Is the binary format compatible across platforms and versions?", "faq.html#faq_format_compat", null ],
      [ "Can I define custom value types?", "faq.html#faq_custom_types", null ],
      [ "Does Container System use memory pooling?", "faq.html#faq_memory_pool", null ],
      [ "How does Container System integrate with the messaging layer?", "faq.html#faq_messaging", null ],
      [ "More Resources", "faq.html#faq_more", null ]
    ] ],
    [ "Troubleshooting Guide", "troubleshooting.html", [
      [ "Serialization Format Mismatch", "troubleshooting.html#trbl_serialization_mismatch", null ],
      [ "SIMD Acceleration Not Available", "troubleshooting.html#trbl_simd_unavailable", null ],
      [ "Thread Safety Violations", "troubleshooting.html#trbl_thread_safety", null ],
      [ "Memory Issues With Large Containers", "troubleshooting.html#trbl_memory_large", null ],
      [ "Type Conversion Errors", "troubleshooting.html#trbl_conversion_errors", null ],
      [ "More Help", "troubleshooting.html#trbl_more", null ]
    ] ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", null ],
        [ "Functions", "namespacemembers_func.html", null ],
        [ "Variables", "namespacemembers_vars.html", null ],
        [ "Typedefs", "namespacemembers_type.html", null ],
        [ "Enumerations", "namespacemembers_enum.html", null ]
      ] ]
    ] ],
    [ "Concepts", "concepts.html", "concepts" ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", "functions_func" ],
        [ "Variables", "functions_vars.html", null ],
        [ "Typedefs", "functions_type.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Functions", "globals_func.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ],
    [ "Examples", "examples.html", "examples" ]
  ] ]
];

var NAVTREEINDEX =
[
"advanced__container__example_8cpp.html",
"classkcenon_1_1container_1_1auto__refresh__reader.html#a4a97674963bd8d069d201e728a30c1bd",
"classkcenon_1_1container_1_1lockfree__container__reader.html#a70bfa6a3e9c179603ddd340e37321291",
"classkcenon_1_1container_1_1value.html#a2ed86e2c68e087e69641b7e3503fb5a3",
"faq.html#faq_format_compat",
"schema__validation__example_8cpp_source.html",
"structkcenon_1_1container_1_1integration_1_1messaging__integration_1_1metrics.html#ad60fc5069ef1e806b79f80af9c5ca4a3",
"variant__value__factory_8h.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';
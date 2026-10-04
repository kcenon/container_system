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
      [ "Samples vs Examples", "md_examples_2README.html#autotoc_md1", null ],
      [ "Examples", "md_examples_2README.html#autotoc_md2", [
        [ "async_coroutine_example", "md_examples_2README.html#autotoc_md3", null ],
        [ "asio_integration_example", "md_examples_2README.html#autotoc_md4", null ],
        [ "messaging_integration_example", "md_examples_2README.html#autotoc_md5", null ]
      ] ],
      [ "Building Examples", "md_examples_2README.html#autotoc_md6", null ],
      [ "Running Examples", "md_examples_2README.html#autotoc_md7", null ],
      [ "Configuration Options", "md_examples_2README.html#autotoc_md8", [
        [ "Messaging Features", "md_examples_2README.html#autotoc_md9", null ],
        [ "Performance Metrics", "md_examples_2README.html#autotoc_md10", null ],
        [ "External Integration", "md_examples_2README.html#autotoc_md11", null ],
        [ "Combined Configuration", "md_examples_2README.html#autotoc_md12", null ]
      ] ],
      [ "Expected Output", "md_examples_2README.html#autotoc_md13", null ],
      [ "Integration with Messaging Systems", "md_examples_2README.html#autotoc_md14", null ]
    ] ],
    [ "Container System Samples", "md_examples_2tutorials_2README.html", [
      [ "Samples vs Examples", "md_examples_2tutorials_2README.html#autotoc_md16", null ],
      [ "Available Samples", "md_examples_2tutorials_2README.html#autotoc_md17", [
        [ "Basic Usage (basic_usage.cpp)", "md_examples_2tutorials_2README.html#autotoc_md18", null ],
        [ "Thread Safety Example (thread_safe_example.cpp)", "md_examples_2tutorials_2README.html#autotoc_md19", null ],
        [ "Performance Benchmark (performance_benchmark.cpp)", "md_examples_2tutorials_2README.html#autotoc_md20", null ],
        [ "Run All Samples (run_all_samples.cpp)", "md_examples_2tutorials_2README.html#autotoc_md21", null ]
      ] ],
      [ "Building the Samples", "md_examples_2tutorials_2README.html#autotoc_md22", [
        [ "Prerequisites", "md_examples_2tutorials_2README.html#autotoc_md23", null ],
        [ "Build Instructions", "md_examples_2tutorials_2README.html#autotoc_md24", null ],
        [ "Alternative Build (samples only)", "md_examples_2tutorials_2README.html#autotoc_md25", null ]
      ] ],
      [ "Sample Output Examples", "md_examples_2tutorials_2README.html#autotoc_md26", [
        [ "Basic Usage Output", "md_examples_2tutorials_2README.html#autotoc_md27", null ],
        [ "Performance Benchmark Results", "md_examples_2tutorials_2README.html#autotoc_md28", null ]
      ] ],
      [ "Understanding the Results", "md_examples_2tutorials_2README.html#autotoc_md29", [
        [ "Performance Metrics", "md_examples_2tutorials_2README.html#autotoc_md30", null ],
        [ "Thread Safety Verification", "md_examples_2tutorials_2README.html#autotoc_md31", null ],
        [ "Memory Usage Analysis", "md_examples_2tutorials_2README.html#autotoc_md32", null ]
      ] ],
      [ "Customizing the Samples", "md_examples_2tutorials_2README.html#autotoc_md33", [
        [ "Adding New Samples", "md_examples_2tutorials_2README.html#autotoc_md34", null ],
        [ "Modifying Benchmarks", "md_examples_2tutorials_2README.html#autotoc_md35", null ]
      ] ],
      [ "Troubleshooting", "md_examples_2tutorials_2README.html#autotoc_md36", [
        [ "Common Issues", "md_examples_2tutorials_2README.html#autotoc_md37", null ],
        [ "Getting Help", "md_examples_2tutorials_2README.html#autotoc_md38", null ]
      ] ],
      [ "License", "md_examples_2tutorials_2README.html#autotoc_md39", null ]
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
    [ "Modules", "modules.html", [
      [ "Modules List", "modules.html", "modules_dup" ],
      [ "Module Members", "modulemembers.html", [
        [ "All", "modulemembers.html", null ],
        [ "Functions", "modulemembers_func.html", null ],
        [ "Variables", "modulemembers_vars.html", null ],
        [ "Typedefs", "modulemembers_type.html", null ],
        [ "Enumerations", "modulemembers_enum.html", null ]
      ] ]
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
        [ "Variables", "functions_vars.html", "functions_vars" ],
        [ "Typedefs", "functions_type.html", null ],
        [ "Enumerations", "functions_enum.html", null ],
        [ "Related Symbols", "functions_rela.html", null ]
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
"classkcenon_1_1container_1_1auto__refresh__reader.html#a3af837f2ecd128a591913f8bc54cf277",
"classkcenon_1_1container_1_1epoch__guard.html#a15b67a27a20e5f5bbcb6daec3ab05cfd",
"classkcenon_1_1container_1_1lockfree__container__reader.html#a1ab429a5b37e41914519590377e8375b",
"classkcenon_1_1container_1_1policy_1_1dynamic__storage__policy.html#a987090ed046b98abfb63c85a3ee4f501",
"classkcenon_1_1container_1_1simd_1_1simd__ops.html#a992b5576f8985ca680e59b99a85382f5",
"classkcenon_1_1container_1_1value__container.html#a024085e13661f2f9ccd59ff83cec1ef5",
"classkcenon_1_1container_1_1value__container.html#a6908a2f491af6eaa0a80114fca24ddf5",
"classkcenon_1_1container_1_1value__container.html#ac3e1e02b2bddeb26af7739da1afefd9d",
"classkcenon_1_1container_1_1value__store.html#a198bf5a57f57dcf9a4a8a0f547235a68",
"container_8cpp.html#adeb94cea0d8fd620660bf01b28c9503c",
"index.html#learning",
"module__kcenon_8container.html#af80387a5bb85c4f2f5f709a82209908c",
"namespacekcenon_1_1container_1_1internal.html#ab27c2320a5125db215224faa5dd45821",
"structkcenon_1_1container_1_1async_1_1detail_1_1executor__awaitable.html#a631479606a92fb8db2c5743c51e42e7e",
"structkcenon_1_1container_1_1internal_1_1fixed__block__pool_1_1statistics.html#a8c807b2855ffabe4ca5ee41e7270b238",
"structkcenon_1_1container_1_1pool__stats.html#aadfbbee7055f734a38265ebab658b864",
"tutorial_serialization.html#tut_ser_simd"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';
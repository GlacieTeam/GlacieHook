add_rules("mode.debug", "mode.release")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git")

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

add_requires("fmt 10.2.1")
add_requires("magic_enum 0.9.7")
add_requires("detours v4.0.1-xmake.1")
add_requires("libhat 2024.9.22")

target("GlacieHook")
    set_kind("static")
    set_languages("cxx20")
    set_symbols("debug")   
    set_exceptions("none")
    add_includedirs("src")
    add_defines(
        "NOMINMAX", 
        "UNICODE",
        "_HAS_CXX23=1",
        "_AMD64_"
    )
    add_cxflags(
        "/utf-8",
        "/EHa"
    )
    add_files("src/**.cpp")
    add_packages(
        "fmt",
        "magic_enum",
        "detours",
        "libhat"
    )
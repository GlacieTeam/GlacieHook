add_rules("mode.debug", "mode.release")

add_repositories("liteldev-repo https://github.com/LiteLDev/xmake-repo.git", "iceblcokmc https://github.com/IceBlcokMC/xmake-repo.git")

if not has_config("vs_runtime") then
    set_runtimes("MD")
end

option("static")
    set_default(false)
    set_showmenu(true)
    set_description("Use static linkage")
option_end()

add_requires("fmt 10.2.1")
add_requires("magic_enum 0.9.7")
add_requires("detours v4.0.1-xmake.1")
add_requires("libhat 2024.9.22")

if not has_config("static") then
    add_requires(
        "endstone 0.6.0",
        "expected-lite 0.8.0",
        "entt 3.14.0",
        "microsoft-gsl 4.0.0",
        "nlohmann_json 3.11.3",
        "boost 1.85.0",
        "glm 1.0.1",
        "concurrentqueue 1.0.4"
    )
end

target("GlacieHook")
    set_kind(has_config("static") and "static" or "shared")
    set_languages("cxx20")
    set_symbols("debug")   
    set_exceptions("none")
    add_headerfiles("src/(**.h)")
    add_includedirs("src")
    add_defines(
        "NOMINMAX", 
        "UNICODE",
        "_AMD64_",
        "_HAS_CXX23=1",
        "GlacieHook_EXPORTS"
    )
    add_cxflags(
        "/utf-8",
        "/EHa"
    )
    add_files("src/ll/**.cpp")
    add_files("src/pl/**.cpp")
    add_packages(
        "fmt",
        "magic_enum",
        "detours",
        "libhat"
    )
    if not has_config("static") then
        add_files("src/endstone/**.cpp")
        add_packages(
            "endstone",
            "expected-lite",
            "entt",
            "microsoft-gsl",
            "nlohmann_json",
            "boost",
            "glm",
            "concurrentqueue"
        )
    end
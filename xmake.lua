target("xalgorithm_test")
    set_languages("c11", "c++20")
    add_rules("mode.debug", "mode.release")
    set_kind("binary")
    
    add_includedirs("include/")
    
    add_headerfiles("include/*.h")
    add_headerfiles("include/*.hpp")
    add_headerfiles("include/*.inl")

    add_files("test/*.cpp")

    if is_mode("debug") then
        -- 添加DEBUG编译宏
        add_defines("DEBUG")
        -- 启用调试符号
        set_symbols("debug")
        -- 禁用优化
        set_optimize("none")
    elseif is_mode("release") then
        -- 隐藏符号
        set_symbols("hidden")
        -- strip所有符号
        set_strip("all")
        -- 开启优化为：最快速度模式
        set_optimize("fastest")
        -- 忽略帧指针
        if is_plat("linux") then
            add_cxflags("-fomit-frame-pointer")
        elseif is_plat("windows") then
            add_cxflags("/Oy")
        end
    end
target_end()
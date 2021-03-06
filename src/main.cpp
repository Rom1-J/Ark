#ifndef ark_ark

#include <chrono>
#include <iostream>

#include <clipp.hpp>
#include <Ark/Ark.hpp>

void bcr(const std::string& file)
{
    try {
        Ark::BytecodeReader bcr;
        bcr.feed(file);
        bcr.display();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

int main(int argc, char** argv)
{
    using namespace clipp;

    enum class mode { help, dev_info, bytecode_reader, version, run };
    mode selected = mode::help;

    std::string file = "";
    bool debug = false;
    std::vector<std::string> wrong;

    auto cli = (
        option("-h", "--help").set(selected, mode::help).doc("Display this message")
        | option("--version").set(selected, mode::version).doc("Display ArkScript version and exit")
        | option("--dev-info").set(selected, mode::dev_info).doc("Display development information and exit")
        | (
            value("file", file).set(selected, mode::run)
            , (
                (
                    option("-d", "--debug").set(debug).doc("Enable debug mode")
                )
                | option("-bcr", "--bytecode-reader").set(selected, mode::bytecode_reader).doc("Launch the bytecode reader")
            )
        )
        , any_other(wrong)
    );

    auto fmt = doc_formatting{}
        .start_column(8)           // column where usage lines and documentation starts
        .doc_column(36)            // parameter docstring start col
        .indent_size(2)            // indent of documentation lines for children of a documented group
        .split_alternatives(true)  // split usage into several lines for large alternatives
    ;

    if (parse(argc, argv, cli) && wrong.empty())
    {
        using namespace Ark;

        switch (selected)
        {
            case mode::help:
                std::cerr << make_man_page(cli, argv[0], fmt).append_section(
                    "LICENSE",
                    "        Mozilla Public License 2.0"
                    ) << std::endl;
                break;
            
            case mode::version:
                std::cout << "Version " << ARK_VERSION_MAJOR << "." << ARK_VERSION_MINOR << "." << ARK_VERSION_PATCH << std::endl;
                break;
            
            case mode::dev_info:
                std::cout << ARK_COMPILER << " " << ARK_COMPILATION_OPTIONS << "\n";
                std::cout << "sizeof(Ark::Value) [VM] = " << sizeof(Ark::internal::Value) << "B\n";
                std::cout << "sizeof(Ark::Frame) [VM] = " << sizeof(Ark::internal::Frame) << "B\n";
                std::cout << "sizeof(Ark::Closure)    = " << sizeof(Ark::internal::Closure) << "B\n";
                std::cout << "sizeof(Ark::VM)         = " << sizeof(Ark::VM) << "B\n";
                std::cout << "sizeof(char)            = " << sizeof(char) << "B\n";
                std::cout << std::endl;
                break;
            
            case mode::run:
            {
                if (debug)
                {
                    Ark::VM_debug vm;
                    vm.doFile(file);
                }
                else
                {
                    Ark::VM vm;
                    vm.doFile(file);
                }
                break;
            }
            
            case mode::bytecode_reader:
                bcr(file);
                break;
        }
    }
    else
    {
        for (const auto& arg : wrong)
            std::cerr << "'" << arg << "'" << " ins't a valid argument" << std::endl;
            
        std::cerr << "Usage:"   << std::endl << usage_lines(cli, argv[0], fmt) << std::endl
                  << "Options:" << std::endl << documentation(cli, fmt) << std::endl
                  << "LICENSE"  << std::endl << "        Mozilla Public License 2.0" << std::endl;
    }

    // to avoid some "CLI glitches"
    std::cout << termcolor::reset;
    
    return 0;
}

#endif

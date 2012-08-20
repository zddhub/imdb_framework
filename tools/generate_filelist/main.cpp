/*
Copyright (C) 2012 Mathias Eitz and Ronald Richter.
All rights reserved.

This file is part of the imdb library and is made available under
the terms of the BSD license (see the LICENSE file).
*/

//#include <QtCore>

#include <util/types.hpp>
#include <util/progress.hpp>
#include <io/cmdline.hpp>
#include <io/filelist.hpp>




// ------------------------------------------------------------
// General usage
//
// create filelist for all png images below a certain rootDir:
// generate_filelist -d imagesRootDir -o filelist -t "*.png"
// ------------------------------------------------------------

using namespace imdb;

class command_files : public Command
{
public:

    command_files()
        : Command("files [options]")
        , _co_rootdir      ("rootdir"      , "d", "root directory of files descriptors are compute from [optional, default is '.']")
        , _co_namefilters  ("namefilters"  , "t", "name filters for files to be listed, e.g. \"*.png\" \"*.jpg\" [required]")
        , _co_filelist     ("filelist"     , "f", "file that contains existing list of filenames [optional, if not provided all matching files in and below rootdir are listed]")
        , _co_outputfile   ("outputfile"   , "o", "output filelist filename [optional, if not provided, output is console.]")
        , _co_randomsample ("random-sample", "r", "random shuffle and truncate file list to given size [optional]")
        , _co_seed         ("seed"         , "s", "seed value for random-sampling [optional, default is current time]")
    {
        add(_co_rootdir);
        add(_co_namefilters);
        add(_co_filelist);
        add(_co_outputfile);
        add(_co_randomsample);
        add(_co_seed);
    }

    bool run(const std::vector<std::string>& args)
    {
        string in_rootdir;
        string in_filelist;
        string in_outputfile;
        size_t in_samples(0);

        warn_for_unknown_option(args);

        // new 31.Oct.2011: rootDir is optional as well, this is helpful
        // if we for example want to modify an existing filelist (e.g.
        // subsampling or outputting to the console).
        in_rootdir = ".";
        if (!_co_rootdir.parse_single<std::string>(args, in_rootdir))
        {
            std::cout << "generate_filelist: no rootdir provided, defaulting to '.'" << std::endl;
        }

        FileList files(in_rootdir);


        // input is an existing filelist
        if (_co_filelist.parse_single<std::string>(args, in_filelist))
        {
            try { files.load(in_filelist); }
            catch(std::exception& e)
            {
                std::cerr << "generate_filelist: failed to load filelist from file " << in_filelist << ": " << e.what() << std::endl;
                return false;
            }
        }

        // input is generated by traversing root directory
        else
        {

            vector<string> in_namefilters;
            if (!_co_namefilters.parse_multiple<string>(args, in_namefilters))
            {
                std::cerr << "generate_filelist: no filetypes argument provided. " << std::endl;
                return false;
            }

            progress_output progress;
            files.lookup_dir(in_namefilters, progress);
            std::cout << "generate_filelist: listed " << files.size() << " files from " << in_rootdir << std::endl;
        }

        if (_co_randomsample.parse_single<size_t>(args, in_samples))
        {
            size_t in_seed(0);
            if (!_co_seed.parse_single<size_t>(args, in_seed))
            {
                //in_seed = QTime::currentTime().msec();
                in_seed = time(0);
            }

            std::cout << "generate_filelist: seed for random sampling: " << in_seed << std::endl;
            files.random_sample(in_samples, in_seed);
        }

        // output is a file
        if (_co_outputfile.parse_single<std::string>(args, in_outputfile))
        {
            try { files.store(in_outputfile); }
            catch (const std::exception& e)
            {
                std::cerr << "generate_filelist: failed to save filelist to " << in_outputfile << ": " << e.what() << std::endl;
                return false;
            }
        }

        // output is console
        else
        {
            for (size_t i = 0; i < files.size(); i++) std::cout << files.get_relative_filename(i) << std::endl;
        }

        return true;
    }

private:

    CmdOption _co_rootdir;
    CmdOption _co_namefilters;
    CmdOption _co_filelist;
    CmdOption _co_outputfile;
    CmdOption _co_randomsample;
    CmdOption _co_seed;
};



int main(int argc, char *argv[])
{
    if (argc <= 1)
    {
        command_files().print();
        return 1;
    }

    return command_files().run(argv_to_strings(argc - 1, &argv[1])) ? 0 : 2;
}

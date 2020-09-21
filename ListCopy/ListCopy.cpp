#include <windows.h>
#include <stdio.h>
#include <algorithm>
#include <set>
#include <string>
#include <getopt.h>
#include <clib.h>

using std::set;
using std::string;
using std::transform;

static bool verbose = false;

void    FileCopy(string const &indir, string const &outdir, string const &filename)
{
    string infile = indir + filename;
    FILE *ifp = fopen(infile.c_str(), "rb");
    if (ifp == NULL)
    {
        return;
    }

    string outfile = outdir + filename;
    char fullOut[MAX_PATH];
    char *filePart;
    GetFullPathName(outfile.c_str(), MAX_PATH, fullOut, &filePart);
    if (filePart)
    {
        *filePart = 0;
    }
    mkdirr(fullOut);

    int ch;
    FILE *ofp = fopen(outfile.c_str(), "rb");
    if (ofp != NULL)
    {
        int ch2;

        do
        {
            ch = getc(ifp);
            ch2 = getc(ofp);
        } while (ch != EOF && ch2 != EOF && ch == ch2);
        fclose(ofp);
        if (ch == EOF && ch2 == EOF)
        {
            fclose(ifp);
            if (verbose)
            {
                printf("Skipping %s: target %s is identical\n", infile.c_str(), outfile.c_str());
            }
            return;
        }
    }

    fseek(ifp, 0, SEEK_SET);
    ofp = fopen(outfile.c_str(), "wb");
    if (ofp == NULL)
    {
        fclose(ifp);
        return;
    }

    printf("Copying %s to %s\n", infile.c_str(), outfile.c_str());

    while ((ch = getc(ifp)) != EOF)
    {
        putc(ch, ofp);
    }

    fclose(ifp);
    fclose(ofp);
}

void FixDirectory(string &directory)
{
    auto fixSlashes = [](char c)
    {
        return c == '/' ? '\\' : c;
    };
    transform(directory.begin(), directory.end(), directory.begin(), fixSlashes);
    if (directory.length() == 0 || directory[directory.length() - 1] != '\\')
    {
        directory += "\\";
    }
}

void    CopyDir(string const &indir, string const &outdir, set<string> const &filenames)
{
    set<string> images;
    set<string> subfolders;

    mkdirr(outdir.c_str());
    for (string const &filename : filenames)
    {
        string infile = indir + filename;
        string outfile = outdir + filename;

        FILE *ifp = fopen(infile.c_str(), "r");
        if (ifp == nullptr)
        {
            continue;
        }
        RAII<FILE *> inFileRaii(ifp, fclose);

        FILE *ofp = fopen(outfile.c_str(), "w");
        if (ofp == nullptr)
        {
            continue;
        }
        RAII<FILE *> outFileRaii(ofp, fclose);

        char buff[256];
        while (fgets(buff, 256, ifp) != nullptr)
        {
            strip(buff);
            if (buff[0] == '@')
            {
                char *backslash = strrchr(buff, '\\');
                char *slash = strrchr(buff, '/');
                char *filename = slash == nullptr ? backslash : backslash == nullptr || backslash < slash ? slash : backslash;
                if (filename == nullptr)
                {
                    continue;
                }
                filename++;
                if (filenames.find(filename) == filenames.end())
                {
                    continue;
                }
                fprintf(ofp, "%s\n", buff);
                string subfolder = string(&buff[1], filename - &buff[1]);
                FixDirectory(subfolder);
                subfolders.insert(subfolder);
            }
            else
            {
                fprintf(ofp, "%s\n", buff);
                char *semicolon = strchr(buff, ';');
                if (semicolon != nullptr)
                {
                    *semicolon = 0;
                }
                images.insert(buff);
            }
        }
    }
    for (string const &image : images)
    {
        FileCopy(indir, outdir, image);
    }
    for (string const &subfolder : subfolders)
    {
        string subindir = indir + subfolder;
        string suboutdir = outdir + subfolder;
        CopyDir(subindir, suboutdir, filenames);
    }
}

int main(int n, char **a)
{
    string indir;
    string outdir;
    set<string> filenames;

    int opt;
    while ((opt = getopt(n, a, "i:o:f:v")) != EOF)
    {
        switch (opt)
        {
        case 'i':
            indir = string(optarg);
            FixDirectory(indir);
            break;
        case 'o':
            outdir = string(optarg);
            FixDirectory(outdir);
            break;
        case 'f':
            filenames.insert(string(optarg));
            break;
        case 'v':
            verbose = true;
            break;
        }
    }

    if (indir == "" || outdir == "" || filenames.size() == 0)
    {
        printf("Usage: ListCopy -i:indirectory -o:outdirectory -f:filename [-v]\n");
        return 1;
    }


    CopyDir(indir, outdir, filenames);

    return 0;
}

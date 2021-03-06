#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
#include <iostream>

struct image_data
{
    std::string name;
    std::string data;
    int length;

    image_data(const std::string &n,
               const std::string &d, int l) :
        name(n), data(d), length(l) {}

	image_data() : length(0) {}
};

int main(int argc, char *argv[])
{
    FILE *fin, *fout;
    int i;
    std::vector<image_data> images;

    if (argc < 3)
        return -1;

    if ((fout = fopen(argv[1], "w"))) {
        fprintf(fout, "// AUTOGENERATED FILE ---- DO NOT TOUCH!\n\n"
                "#include \"SDL.h\"\n"
                "#include <map>\n"
                "#include <string>\n\n"
                "struct image_data {\n\tint length;\n\tunsigned char *data;\n"
				"\timage_data(int s, unsigned char *d) : length(s), data(d) {}\n"
				"\timage_data() : length(0) {}\n};\n\n"
                "typedef std::map<std::string, image_data> imagemap;\n\n"
               );

        for (int i = 2; i < argc; i++) {
            if ((fin = fopen(argv[i], "rb"))) {
                int c;
                image_data temp;
                char name[256], *n;

                if ( (n = strrchr(argv[i],'/'))) {
                    strcpy(name, n + 1);
                }
                else
                    strcpy(name, argv[i]);

                temp.name = name;

                n = name;

                while (*n) {
                    if (*n == '.')
                        *n = '_';
                    n++;
                }

                temp.data = name;

                fseek(fin, 0, SEEK_END);
                temp.length = ftell(fin);
				std::cerr << "Adding to " << argv[1] << " image " << temp.name << " of " << temp.length << " bytes.\n";
                fseek(fin, 0, SEEK_SET);

                images.push_back(temp);

                fprintf(fout, "\n// %s\n", temp.name.c_str());
                fprintf(fout, "static unsigned char %s[] = {\n", temp.data.c_str());

                int line = 0;

                while ((c = fgetc(fin)) >= 0) {
                    fprintf(fout, "%d,", c);
                    line ++;

                    if (line > 40) { // max 40 numbers per line
                        line = 0;
                        fputc('\n', fout);
                    }
                }
                fputs("};\n", fout);

                fclose(fin);

        	}

        }

        fprintf(fout, "\nstatic imagemap::value_type values[] = {\n");
        
        for (std::vector<image_data>::iterator it = images.begin(); it != images.end(); ++it) {
            fprintf(fout, "\timagemap::value_type(\"%s\", image_data(%d,%s)),\n",
                                it->name.c_str(), it->length, it->data.c_str());
        }

        fprintf(fout, "};\n\n"
                      "static imagemap images(values, values + %d);\n\n", images.size());

		fprintf(fout, 
				"SDL_RWops *load_or_grab_image(const std::string &name) {\n"
				"\tstd::map<std::string, image_data>::iterator it;\n\tstd::string n;\n"
				"\tstd::string::size_type p = name.find_last_of(\"/\");\n"
				"\tif (p != std::string::npos) n = name.substr(p + 1); else n = name;\n\n"
				"\tif ( (it = images.find(n)) != images.end())\n"
				"\t\treturn SDL_RWFromMem(it->second.data, it->second.length);\n\n"
				"\treturn SDL_RWFromFile(name.c_str(), \"rb\");\n}\n");

        fclose(fout);
    }
	else
		return -2;

    return 0;
}

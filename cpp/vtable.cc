#include <iostream>
#include <map>
#include <cxxabi.h>
#include <cstdlib>
#include <cerrno>
#include <cstdio>
#include <cstring>

#include <fcntl.h>
#include <link.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>


class animal {
  protected:
   int weight;
  public:
   virtual const char *getname() = 0;
   virtual bool warmblooded() = 0;
   int getweight();
   virtual ~animal();
   animal(int weight_);
};

class mammal : virtual public animal {
  protected:
   bool will_bite;
  public:
   virtual bool warmblooded();
   virtual ~mammal();
   bool bites();
   mammal(int weight_, bool will_bite_);
};

class aquatic : virtual public animal {
  protected:
   int swim_speed;
  public:
   int swimspeed();
   aquatic(int weight_, int swim_speed_);
   virtual ~aquatic();   
};

class dolphin : public mammal, public aquatic {
  public:
   dolphin();
   virtual ~dolphin();
   virtual const char *getname();
};

class elephant : public mammal {
public:
   elephant();
   virtual ~elephant();
   virtual const char *getname();
};

animal::animal(int weight_) :
   weight(weight_)
{
}

animal::~animal() {
}

int animal::getweight() {
   return weight;
}

mammal::mammal(int weight_, bool will_bite_) :
   animal(weight_),
   will_bite(will_bite_)
{
}

mammal::~mammal()
{
}

bool mammal::warmblooded()
{
   return true;
}

bool mammal::bites()
{
   return will_bite;
}

aquatic::aquatic(int weight_, int swim_speed_) : 
   animal(weight_),
   swim_speed(swim_speed_)
{
}

aquatic::~aquatic()
{
}

int aquatic::swimspeed()
{
   return swim_speed;
}

dolphin::dolphin() :
   animal(100),
   mammal(100, false),
   aquatic(100, 50)
{
}

dolphin::~dolphin()
{
}

elephant::elephant() :
   animal(1000000000),
   mammal(1000000000, false)
{
}

elephant::~elephant()
{
}

const char *elephant::getname()
{
   return "elephant";
}

const char *dolphin::getname()
{
   return "dolphin";
}

static std::map<void *, std::string> symnames;
static bool fill_in_symbols(char *aout)
{
   
   int fd = open(aout, O_RDONLY);
   if (fd == -1) {
      fprintf(stderr, "Could not open %s: %s\n", aout, strerror(errno));
      return false;
   }
   struct stat buf;
   int result = fstat(fd, &buf);
   if (result == -1) {
      fprintf(stderr, "Could not stat executable: %s\n", strerror(errno));
      close(fd);      
      return false;
   }
   void *load_addr = mmap(NULL, buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
   if (load_addr == MAP_FAILED) {
      fprintf(stderr, "Could not mmap executable: %s\n", strerror(errno));
      close(fd);
      return false;
   }
   unsigned char *base = (unsigned char *) load_addr;

   ElfW(Ehdr) *ehdr = (ElfW(Ehdr *)) load_addr;
   ElfW(Shdr) *sections = (ElfW(Shdr) *) (base + ehdr->e_shoff);

   for (ElfW(Half) i = 0; i < ehdr->e_shnum; i++) {
      ElfW(Shdr) *sec = sections + i;
      if (sec->sh_type != SHT_SYMTAB)
         continue;
      ElfW(Sym) *syms = (ElfW(Sym) *) (base + sec->sh_offset);
      char *names = (char *) (base + sections[sec->sh_link].sh_offset);
      for (size_t j = 0; j < (sec->sh_size / sizeof(ElfW(Sym))); j++) {
         char *name = syms[j].st_name + names;
         ElfW(Addr) addr = syms[j].st_value;

         int status;
         char *demangled_name = abi::__cxa_demangle(name, 0, 0, &status);
         char *aname = status == 0 ? strdup(demangled_name) : name;
         symnames[(void *) addr] = std::string(aname);
      }
   }
   close(fd);
   return true;
}

static void print_vtable(const char *classname, void *o)
{
   void ***obj = static_cast<void ***>(o);
   for (int i = 0; ; i++) {
      std::map<void *, std::string>::iterator j = symnames.find((*obj)[i]);
      if (j == symnames.end())
         break;
      else
         std::cout << classname << " vtable[" << i << "] = " << j->second << "\n";
   }
}

static void print_animal(animal *a)
{
   std::cout << "Did you know that a " << a->getname() << " maybe weighs " << a->getweight() << " pounds? They might also be " <<( a->warmblooded() ? "warm blooded" : "cold blooded") << ".\n";   
}

int main(int argc, char *argv[])
{
   bool result = fill_in_symbols(argv[0]);
   if (!result)
      return -1;
   
   dolphin *d = new dolphin();
   elephant *e = new elephant();

   print_animal(d);
   print_animal(e);
   
   print_vtable("dolphin", d);
   print_vtable("elephant", e);
   print_vtable("(mammal *) dolphin", static_cast<mammal *>(d));
   print_vtable("(mammal *) elephant", static_cast<mammal *>(e));
   print_vtable("(aquatic *) dolphin", static_cast<aquatic *>(d));
   print_vtable("(animal *) dolphin", static_cast<animal *>(d));         
   print_vtable("(animal *) elephant", static_cast<animal *>(e));      
   
   delete d;
   delete e;
   return 0;
}

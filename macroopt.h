/*
  The MIT License (MIT)
  
  Copyright (c) 2015 sebbeliten
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#ifndef MACROOPT_H
#define MACROOPT_H

#include <stdio.h>

/*
example of usage (note that this might not look pretty on auto indendation),

  #include "macroopts.h"
  int
  main (int argc, char **argv)
  {
    // load everything onto the stack
    {
      int n = 1;
      macroopt
      case 'h':
        puts ("No Help Available!");
        break;
      case 'v':
        puts ("Version 1.0");
        break;
      case 'f':gotarg
        puts (optarg);
        break;
      case '-':
        macroopt_optyes = 0; //stop processing options when '--' is found
        break;
      done
      else argv[n++] = argv[optind]; //sort positional arguments in reversed order
      int posend = n;
      while (--n)
        puts (argv[n]);
      //chronological order
      for (n = 1; n < posend; ++n)
        puts (argv[n]);
    }
    // popped stack
    return 0;
  }

Arguments are interpreted according to defined macros, here's *gotarg*,
  
    -jfFILE | -j -f FILE | -j -fFILE | -fFILE -j | -f FILE -j | ETC...
    
    -f-f    //"-f"'s argument will be "-f"
    -f -f   //error (no argument for "-f")
*/


/*
    the logic behind this is to leave *macroopt_pointer* at an address
    whose next address points to a null terminator character. this is needed
    in order to execute the user instructions between a *case* keyword and a *break* keyword
    within the *switch* without forcing the use of a *goto* keyword on the user (to get to the next option).
    it's possible to replace the last 'else' and 'while' statement with, 
      macroopt_pointer = "\0" //note: "\0" = \0\0
    to also *trick* the following loop to exit (the first character can be 
    anything (it's ignored on next iteration)). i just found exactly that kind of redundant. 
    to the CPU this does not matter though. the *redundant* way is infact cheaper
    on processing power (and probably on resulting binary size too?) but i'm not an assembly enlightened.
    i'm actually gonna switch to that *redundant* way and leave this here for historical reasons,      
      else \
        while (*(macroopt_pointer+1) != '\0') \
          ++macroopt_pointer;
    actually, i don't really care. maybe i'll do it later.
    i did it! formally, the variable is called: macroopt_null_terminator
*/

// option got argument, but argument cannot begin with '-' (unless immediately followed)
#define gotarg \
  optarg = macroopt_pointer + 1; \
  if (*optarg == '\0') \
  { \
    if (optind+1 < argc && argv[optind+1][0] != '-') \
      optarg = argv[++optind]; \
    else \
      goto macroopt_err_arg; \
  } \
  else \
    while (*(macroopt_pointer+1) != '\0') \
      ++macroopt_pointer;

// how to differentiate an option from an argument? (variable amount arguments)
#define nxtopt \
  if (optind+1 < argc && argv[optind+1][0] != '-') \
    optarg = argv[++optind]; \
  else \
    goto macroopt_err_arg;

//specify option requires argument (allow '-'.* arguments)
/*
#define require \
  optarg = macroopt_pointer + 1; \
  if (*optarg == '\0') \
  { \
    if (++optind == argc) \
      goto macroopt_err_arg; \
    else \
      optarg = argv[optind]; \
  } \
  macroopt_pointer = macroopt_null_terminator;
*/

// like *gotarg* but ignore -.*
#define gotarganyway

// option *can* have an argument.
#define maygotarg \
  optarg = macroopt_pointer + 1; \
  if (*optarg == '\0') \
  { \
    if (optind+1 < argc && argv[optind+1][0] != '-') \
      optarg = argv[++optind]; \
    else \
      optarg = NULL; \
  } \
  macroopt_pointer = macroopt_null_terminator;

/*
    optind           - index into argv
    optarg           - argument
    macroopt_optyes  - whether to process options (useful for "--")
    macroopt_pointer - just a pointer to cycle through potential consecutive sequences of options
*/

/*
    begin processing argv[]
*/
#define macroopt \
int   optind                      = 1; \
char *optarg                      = NULL; \
int   macroopt_optyes             = 1; \
char *macroopt_pointer            = argv[1]; \
char *macroopt_null_terminator    = "\0"; \
if (0) \
{ \
  macroopt_err_arg: \
    fprintf(stderr,"option '%c' needs argument\n",*macroopt_pointer); \
    return 1; \
  macroopt_err_opt: \
    fprintf(stderr,"option '%c' is undefined\n",*macroopt_pointer); \
    return 1; \
} \
for (; optind < argc; macroopt_pointer = argv[++optind]) \
  if (macroopt_optyes && *macroopt_pointer == '-') \
  { \
    while (++macroopt_pointer) \
      switch (*macroopt_pointer) \
      {
/*
    get next option in argv[]
*/
#define done \
      default: \
        if (*macroopt_pointer != '\0') \
          goto macroopt_err_opt; \
        else \
          goto macroopt_next; \
        break; \
      } \
    macroopt_next:; \
  }

#endif

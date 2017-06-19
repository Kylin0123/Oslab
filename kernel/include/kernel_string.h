/* Copyright (C) 1994 DJ Delorie, see COPYING.DJ for details */
#include <string.h>
 
char *strtok(char *s,const char *delim) 
{
    static char *last;
    char *tok;
    char *ucdelim;
    char *spanp;
    int c,sc;

    /*s为空，并且上次剩余值也为空，则直接返回NULL，否则s为last或当前值中有值的一方*/
    if (s == NULL && (s = last) == NULL)
        return NULL;
  
    
    int found = 0;//是否找到与delim匹配的字符
    
    //处理连续的待匹配的字符
    cont:
    c=*s++;
    for (spanp = (char *)delim;(sc = *spanp++) != 0;)
    {
        if (c == sc)
            goto cont;
    }
    if (c == 0) 
    {
        last = NULL;
        return NULL;
    }

    tok = s-1;
    while (!found && *s != '\0') 
    {
        ucdelim = (char *) delim;
        while (*ucdelim) 
        {
            if (*s == *ucdelim) 
            {
                found = 1;
                *s = '\0';
                last = s + 1;
                break;
            }
            ucdelim++;
        }
        if (!found)
        {
            s++;
            if(*s=='\0')
                last = NULL;
        }
    }

    return tok;
}

/*char* strtok(char *s, const char *delim)
{
  const char *spanp;
  int c, sc;
  char *tok;
  static char *last;
 
 
  if (s == NULL && (s = last) == NULL)
    return (NULL);
 

 cont:
  c = *s++;
  for (spanp = delim; (sc = *spanp++) != 0;) {
    if (c == sc)
      goto cont;
  }
 
  if (c == 0) {                
    last = NULL;
    return (NULL);
  }
  tok = s - 1;
 
  for (;;) {
    c = *s++;
    spanp = delim;
    do {
      if ((sc = *spanp++) == c) {
        if (c == 0)
          s = NULL;
        else
          s[-1] = 0;
        last = s;
        return (tok);
      }
    } while (sc != 0);
  }
}*/

int mystrcmp(const char *dest, const char *source)  
{  
   assert((NULL != dest) && (NULL != source));  
   while (*dest && *source && (*dest == *source))  
           {  
                    dest ++;  
                   source ++;  
           }  
   return *dest - *source;  
}  

char *mystrcat(char *str1, char *str2)  
{  
    assert((str1!=NULL)&&(str2!=NULL));  
    char *pt = str1;  
    while(*str1!='\0') str1++;  
    while(*str2!='\0') *str1++ = *str2++;  
    *str1 = '\0';  
    return pt;  
}  
 

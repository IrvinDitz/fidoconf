#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>

#include "fidoconfig.h"

//#define USEREGEXP 1

#define ADDREXPRESSION "[0-9]{1,5}:[0-9]{1,5]/[0-9]{1,5}(\\.[0-9]{0,5})?(@[a-z\\.])?"

int testExpression(char *expr, char *str)
{
#ifdef USEREGEXP
   regex_t cExpr;
   int rc;
   rc = regcomp(&cExpr, expr, REG_ICASE | REG_NOSUB);
   if (rc!=0) return rc;
   rc = regexec(&cExpr, str, 0, NULL, 0);

   regfree(&cExpr);
   return rc;
#else
   return 0;
#endif
}

char *getRestOfLine() {
   return stripLeadingChars(strtok(NULL, "\0"), " \t");
}

int parseVersion(char *token, s_fidoconfig *config)
{
   char buffer[10], *temp = token;
   int i = 0;

   // test
   i = testExpression("[0-9]+\\.[0-9]+", token);
   if (i!= 0 ) return i;

   while (isdigit(*temp)) {
      buffer[i] = *temp;
      i++; temp++;
   }
   buffer[i] = 0;

   config->cfgVersionMajor = atoi(buffer);

   temp++; // eat .
   i = 0;

   while (isdigit(*temp)) {
      buffer[i] = *temp;
      i++; temp++;
   }
   buffer[i] = 0;

   config->cfgVersionMinor = atoi(buffer);

   return 0;
}

int parseName(char *token, s_fidoconfig *config)
{
   config->name = (char *) malloc(strlen(token)+1);
   strcpy(config->name, token);
   return 0;
}

int parseLocation(char *token, s_fidoconfig *config)
{
   config->location = (char *) malloc(strlen(token)+1);
   strcpy(config->location, token);
   return 0;
}

int parseSysop(char *token, s_fidoconfig *config)
{
   config->sysop = (char *) malloc(strlen(token)+1);
   strcpy(config->sysop, token);
   return 0;
}

int parseAddress(char *token, s_fidoconfig *config)
{
   char *aka;
   int rc = testExpression(ADDREXPRESSION, token);
   if (rc!=0) return rc;

   aka = strtok(token, " \t"); // only look at aka

   config->addr = realloc(config->addr, sizeof(s_addr)*(config->addrCount+1));
   string2addr(aka, &(config->addr[config->addrCount]));
   config->addrCount++;
   
   return 0;
}

int parsePath(char *token, char **var)
{
   char limiter;
#ifdef UNIX
   limiter = '/';
#else
   limiter = '\\';
#endif
   if (token[strlen(token)-1] == limiter) {
      *var = (char *) malloc(strlen(token)+1);
      strcpy(*var, token);
   } else {
      *var = (char *) malloc(strlen(token)+2);
      strcpy(*var, token);
      (*var)[strlen(token)] = limiter;
      (*var)[strlen(token)+1] = '\0';
   }
      
   return 0;
}

int parsePublic(char *token, s_fidoconfig *config)
{
   char limiter;
   
   if (token == NULL) return 1;
   config->public = realloc(config->public, sizeof(char *)*config->publicCount);

#ifdef UNIX
   limiter = '/';
#else
   limiter = '\\';
#endif

   if (token[strlen(token)-1] == limiter) {
      config->public[config->publicCount] = (char *) malloc(strlen(token)+1);
      strcpy(config->public[config->publicCount], token);
   } else {
      config->public[config->publicCount] = (char *) malloc(strlen(token)+2);
      strcpy(config->public[config->publicCount], token);
      (config->public[config->publicCount])[strlen(token)] = limiter;
      (config->public[config->publicCount])[strlen(token)+1] = '\0';
   }
      
   config->publicCount++;
   return 0;
}

int parseAreaOption(s_fidoconfig config, char *option, s_area *area)
{
   char *error;
   char *token;
   int rc;
   
   if (stricmp(option, "p")==0) {
      area->purge = strtol(strtok(NULL, " \t"), &error, 0);
      if ((error != NULL) && (*error != '\0')) return 1;     // error occured;
   }
   else if (stricmp(option, "m")==0) {
      area->max = strtol(strtok(NULL, " \t"), &error, 0);
      if ((error != NULL) && (*error != '\0')) return 1;     // error
   }
   else if (stricmp(option, "nodc")==0) area->noDC = 1;
   else if (stricmp(option, "a")==0) {
      token = strtok(NULL, " \t");
      rc = testExpression(ADDREXPRESSION, token);
      if (rc != 0) return rc;
      area->useAka = getAddr(config, token);
      if (area->useAka == NULL) return 1;
   }
   else if (stricmp(option, "tinysb")==0) area->tinySB = 1;
   else if (stricmp(option, "h")==0) area->hide = 1;
   else if (stricmp(option, "manual")==0) area->manual = 1;
   else if (stricmp(option, "nopause")==0) area->noPause = 1;
   else if (stricmp(option, "dupeCheck")==0) {
      token = strtok(NULL, " \t");
      if (stricmp(token, "off")==0) area->dupeCheck = off;
      else if (stricmp(token, "move")==0) area->dupeCheck = move;
      else if (stricmp(token, "del")==0) area->dupeCheck = del;
      else return 1; // error
   }
   else if (stricmp(option, "dupehistory")==0) {
      area->dupeHistory = strtol(strtok(NULL, " \t"), &error, 0);
      if ((error != NULL) && (*error != '\0')) return 1;    // error
   }
      
   return 0;
}

int parseArea(s_fidoconfig config, char *token, s_area *area)
{
   char *tok;
   int rc = 0;
   
   memset(area, 0, sizeof(s_area));

   area->msgbType = MSGTYPE_SDM;
   area->useAka = &(config.addr[0]);

   tok = strtok(token, " \t");
   if (tok == NULL) return 1;         // if there is no areaname

   area->areaName= (char *) malloc(strlen(tok)+1);
   strcpy(area->areaName, tok);

   tok = strtok(NULL, " \t");
   if (tok == NULL) return 2;         // if there is no filename
   area->fileName = (char *) malloc(strlen(tok)+1);
   strcpy(area->fileName, tok);

   while ((tok = strtok(NULL, " \t"))!= NULL) {
      if (stricmp(tok, "Squish")==0) area->msgbType = MSGTYPE_SQUISH;
      else if(isdigit(tok[0])) {
         area->downlinks = realloc(area->downlinks, sizeof(s_link*)*(area->downlinkCount+1));
         area->downlinks[area->downlinkCount] = getLink(config, tok);
         if (area->downlinks[area->downlinkCount] == NULL) return 1;
         area->downlinkCount++;
      }
      else if(tok[0]=='-') rc = parseAreaOption(config, tok+1, area);          // parseOption without leading -
   }
   
   return rc;
}

int parseEchoArea(char *token, s_fidoconfig *config)
{
   int rc;
   
   config->echoAreas = realloc(config->echoAreas, sizeof(s_area)*(config->echoAreaCount+1));
   rc = parseArea(*config, token, &(config->echoAreas[config->echoAreaCount]));
   config->echoAreaCount++;
   return rc;
}

int parseLink(char *token, s_fidoconfig *config)
{
   config->links = realloc(config->links, sizeof(s_link)*(config->linkCount+1));
   memset(&(config->links[config->linkCount]), 0, sizeof(s_link));
   config->links[config->linkCount].name = (char *) malloc (strlen(token)+1);
   strcpy(config->links[config->linkCount].name, token);

   // if handle not given use name as handle
   if (config->links[config->linkCount].handle == NULL) config->links[config->linkCount].handle = config->links[config->linkCount].name;
   if (config->links[config->linkCount].ourAka == NULL) config->links[config->linkCount].ourAka = &(config->addr[0]);
   
   config->linkCount++;
   return 0;
}

int parsePWD(char *token, char **pwd) {
   *pwd = (char *) malloc(9);
   strncpy(*pwd, token, 8);        // only use 8 characters of password
   (*pwd)[8] = '\0';
   if (strlen(token)>8) return 1;
   else return 0;
}

int parseHandle(char *token, s_fidoconfig *config) {
   if (token == NULL) return 1;

   config->links[config->linkCount-1].handle = (char *) malloc (strlen(token)+1);
   strcpy(config->links[config->linkCount-1].handle, token);
   return 0;
}

int parseRoute(char *token, s_fidoconfig *config, s_route **route, UINT *count) {
   char *option;
   int  rc = 0;
   
   *route = realloc(*route, sizeof(s_route)*(*count+1));
   memset(route[*count], 0, sizeof(s_route));

   option = strtok(token, " \t");
   
   while (option != NULL) {
      if (stricmp(option, "enc")==0) (*route)->enc = 1;
      else if (stricmp(option, "noenc")==0) (*route)->enc = 0;
      else if (stricmp(option, "hold")==0) (*route)->flavour = hold;
      else if (stricmp(option, "normal")==0) (*route)->flavour = normal;
      else if (stricmp(option, "crash")==0) (*route)->flavour = crash;
      else if (stricmp(option, "direct")==0) (*route)->flavour = direct;
      else if (stricmp(option, "immediate")==0) (*route)->flavour = immediate;
      else if (stricmp(option, "hub")==0) (*route)->routeVia = hub;
      else if (stricmp(option, "host")==0) (*route)->routeVia = host;
      else if (stricmp(option, "boss")==0) (*route)->routeVia = boss;
      else if (stricmp(option, "noroute")==0) (*route)->routeVia = noroute;
      else if (isdigit(option[0])) {
         if (((*route)->routeVia == 0) && ((*route)->target == NULL))
            (*route)->target = getLink(*config, option);
         else strcpy((*route)->pattern, option);
         if ((*route)->target == NULL) rc = 2;
      }
      option = strtok(NULL, " \t");
   }

   (*count)++;
   return rc;
}

int parseLine(char *line, s_fidoconfig *config)
{
   char *token, *temp;
   int rc = 0;

   temp = (char *) malloc(strlen(line)+1);
   strcpy(temp, line);
      
   token = strtok(temp, " \t");

   //printf("Parsing: %s\n", line);
   //printf("token: %s - %s\n", line, strtok(NULL, "\0"));
   if (stricmp(token, "version")==0) rc = parseVersion(getRestOfLine(), config);
   else if (stricmp(token, "name")==0) rc = parseName(getRestOfLine(), config);
   else if (stricmp(token, "location")==0) rc = parseLocation(getRestOfLine(), config);
   else if (stricmp(token, "sysop")==0) rc =parseSysop(getRestOfLine(), config);
   else if (stricmp(token, "address")==0) rc = parseAddress(getRestOfLine(), config);
   else if (stricmp(token, "inbound")==0) rc = parsePath(getRestOfLine(), &(config->inbound));
   else if (stricmp(token, "protinbound")==0) rc = parsePath(getRestOfLine(), &(config->protInbound));
   else if (stricmp(token, "listinbound")==0) rc = parsePath(getRestOfLine(), &(config->listInbound));
   else if (stricmp(token, "localinbound")==0) rc= parsePath(getRestOfLine(), &(config->localInbound));
   else if (stricmp(token, "outbound")==0) rc = parsePath(getRestOfLine(), &(config->outbound));
   else if (stricmp(token, "public")==0) rc = parsePublic(getRestOfLine(), config);
   else if (stricmp(token, "logFileDir")==0) rc = parsePath(getRestOfLine(), &(config->logFileDir));
   else if (stricmp(token, "dupeHistoryDir")==0) rc = parsePath(getRestOfLine(), &(config->dupeHistoryDir));
   else if (stricmp(token, "nodelistDir")==0) rc = parsePath(getRestOfLine(), &(config->nodelistDir));
   else if (stricmp(token, "magic")==0) rc = parsePath(getRestOfLine(), &(config->magic));
   else if (stricmp(token, "netmailArea")==0) rc = parseArea(*config,getRestOfLine(),&(config->netMailArea));
   else if (stricmp(token, "dupeArea")==0) rc = parseArea(*config, getRestOfLine(), &(config->dupeArea));
   else if (stricmp(token, "badArea")==0) rc = parseArea(*config, getRestOfLine(), &(config->badArea));
   else if (stricmp(token, "echoArea")==0) rc = parseEchoArea(getRestOfLine(), config);
   else if (stricmp(token, "link")==0) rc = parseLink(getRestOfLine(), config);
   else if (stricmp(token, "password")==0) {
      rc = parsePWD(getRestOfLine(), &(config->links[config->linkCount-1].defaultPwd));
      // if another pwd is not known (yet), make it point to the defaultPWD
      if (config->links[config->linkCount-1].pktPwd == NULL) config->links[config->linkCount-1].pktPwd = config->links[config->linkCount-1].defaultPwd;
      if (config->links[config->linkCount-1].ticPwd == NULL) config->links[config->linkCount-1].ticPwd = config->links[config->linkCount-1].defaultPwd;
      if (config->links[config->linkCount-1].areaFixPwd == NULL) config->links[config->linkCount-1].areaFixPwd = config->links[config->linkCount-1].defaultPwd;
      if (config->links[config->linkCount-1].fileFixPwd == NULL) config->links[config->linkCount-1].fileFixPwd = config->links[config->linkCount-1].defaultPwd;
      if (config->links[config->linkCount-1].bbsPwd == NULL) config->links[config->linkCount-1].bbsPwd = config->links[config->linkCount-1].defaultPwd;
   }
   else if (stricmp(token, "aka")==0) {
      string2addr(getRestOfLine(), &(config->links[config->linkCount-1].hisAka));
      rc = 0;
   }
   else if (stricmp(token, "ouraka")==0) {
      rc = 0;
      config->links[config->linkCount-1].ourAka = getAddr(*config, getRestOfLine());
      if (config->links[config->linkCount-1].ourAka == NULL) rc = 1;
   }
   else if (stricmp(token, "pktpwd")==0) rc = parsePWD(getRestOfLine(), &(config->links[config->linkCount-1].pktPwd));
   else if (stricmp(token, "ticpwd")==0) rc = parsePWD(getRestOfLine(), &(config->links[config->linkCount-1].ticPwd));
   else if (stricmp(token, "araefixpwd")==0) rc = parsePWD(getRestOfLine(), &(config->links[config->linkCount-1].areaFixPwd));
   else if (stricmp(token, "filefixpwd")==0) rc = parsePWD(getRestOfLine(), &(config->links[config->linkCount-1].fileFixPwd));
   else if (stricmp(token, "bbspwd")==0) rc = parsePWD(getRestOfLine(), &(config->links[config->linkCount-1].bbsPwd));
   else if (stricmp(token, "handle")==0) rc = parseHandle(getRestOfLine(), config);
   else if (stricmp(token, "route")==0) rc = parseRoute(getRestOfLine(), config, &(config->route), &(config->routeCount));
   else if (stricmp(token, "routeFile")==0) rc = parseRoute(getRestOfLine(), config, &(config->routeFile), &(config->routeFileCount));
   else if (stricmp(token, "routeMail")==0) rc = parseRoute(getRestOfLine(), config, &(config->routeMail), &(config->routeMailCount));
                                                          
   if (rc != 0) {
      printf("Error %d in: %s\n", rc, line);
      return rc;
   }

   return 0;
}

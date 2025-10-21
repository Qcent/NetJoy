///  
/// DATA
///  
unsigned int HOLIDAY_FLAG = 1;

#define VALENTINES_DAY    214
#define ST_PATRICKS_DAY   317
#define APRIL_TWENTY_DAY  420
#define CANADA_DAY		  701
#define NEVER_FORGET_DAY  911
#define HALLOWEEN_DAY	 1031
#define CHRISTMAS_DAY	 1225

static const std::unordered_set<int> g_HOLIDAYS = {
    VALENTINES_DAY,
    ST_PATRICKS_DAY,
    APRIL_TWENTY_DAY,
    CANADA_DAY,
    NEVER_FORGET_DAY,
    HALLOWEEN_DAY,
    CHRISTMAS_DAY
};

///  
/// ART
///  

/*
//	--- HALLOWEEN
*/
wchar_t PUMPKIN_LARGE[529] = L"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t█▒▓██\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t▓▒▒▒█\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t███░████▓█▒██\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t█▒██▓▓██▓█▓█▒███▓██▓█\t\t\t\t\t\t\t\
\t\t\t█▒█▒  ▓█▒ ▒█▓▒██ ▒██  ██▒█\t\t\t\t\
\t\t▓██ ░░▓▒░ ▓▓ ░░░░█▒  █▓ ░█▒█\t\t\t\
\t█▓█ ░░▓██  ▒░░░░░░░▓██▒▓▒  █▒█\t\t\
█░█░░ ▓░████ ░░░░░ █████▒▓░ ▒█ █\t\
█▓█ ░░▒█████▓▒██▓░██████▒ ░░ █░█\t\
██▓  ▓█▓  ░ ▓████▓     ▓█▓ ░ █▒█\t\
\t██ ░░██▓█░░▒░░░░  ▒██▒██ ░░░█▒\t\t\
\t██▓ ▒░█████▒░░░░▓██▓▓██░▓░ ███\t\t\
\t\t██▓ ▓▒ ▓█████▓████░▓░ █▓ ██\t\t\t\t\
\t\t\t\t\t█▒▓▓  ░░░▓▓▓▒░▒░░ █▓██\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t████   ▓█▓███\t\t\t\t\t\t\t\t\t\t";

wchar_t SKULL_LARGE[595] = L"\t\t\t\t\t\t\t\t\t\t▓░░          ▓\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t░░                  \t\t\t\t\t\t\
\t\t\t\t\t░░                      \t\t\t\t\
\t\t\t\t▓░              ▒         ▓\t\t\
\t\t\t▒        ░▒░░░            ░░▓\t\
\t\t▓██▒░   ▒████▓░  ▒░░  ░   ░░░░\t\
\t▓█████  ░███████▓  ░░░ ▒   ░░▒▒▓\
\t▓█▓▓▓  ░▒████▓▓▓▓  ▓▒░░    ░░▓▒▓\
\t▓█▓▓█▓ ░ ▒███▓▓▓   ▒░░░    ▒▒▒▒▓\
█░  ▒▓▓▓    ░        ░░░    ▒▒▒▓\t\
░▒░▒▓  ▓      ░ ░▒▒▒░▒░▓░ ░░▒▓█\t\t\
\t\t░░      ░ ░▒████████░░▓▒▓▓▓█\t\t\t\
\t\t▒  ░ ░░ ▒▓▓░███▓▓▒▒▒░▒█▓██\t\t\t\t\t\
\t\t░░ ▒▓▒▒ ░░▓███ ▒░░░ ▒\t\t\t\t\t\t\t\t\t\t\
\t\t▓▒ ░ ░  ▓   ▓  ░ ░▒\t\t\t\t\t\t\t\t\t\t\t\t\
\t▓▒░░░         ░   ░░▓\t\t\t\t\t\t\t\t\t\t\t\
\t▒▒░ ░░░░░░▒▒▒▒▒░▒▒▒█\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t▓▒▒▒▓█\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

wchar_t GHOST_LARGE[595] = L"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t▒▒  ░\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t▓▒▒▒   ░░░\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t▓▒▒▒       ▓\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t▓          ▒\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t ██   ██    ░\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t▓   ██        ░\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t▓               ░\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t▓        ▒       ▓\t\t\t\t\t\t\t\t\t\t\
\t\t\t▓        ▒▒  ▒           ▒\t\t\t\t\
▓▒▒▒       ▓  ▒▒    ▓ ▒       \t\t░\
 \t\t\t▒▒                 ▒▒▒▒▒▒▒▒▓\t\
\t\t\t\t\t\t▓                  ▒▒▒▓\t\t\t\t\
\t\t\t\t\t\t\t\t▓        ▒▒       ▒▓\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t▓ ▒▒     ▒▒▒▒░   ░\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t░░▒     ▒\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t▓ ░\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

wchar_t BAT_MEDIUM[127] = L"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t██\t\t\t█\t█\t\t\t██\t\t\t\t\
\t\t██▓███\t███\t███▓██\t\t\
\t██▓▒▓████▓████▓▒▓██\t\
██▓█▓██\t█▓░▓█\t██▓█▓██\
\t█\t█\t█\t\t\t█\t█\t\t\t█\t█\t█\t";

wchar_t CREEP_MEDIUM[211] = L"\t\t\t\t░░▒\t░▓▓▓▒░░░ \t\t\t\t\
\t\t▓░░░▒▒▒░░░▒▒▒▒░░▓\t\t\
\t▓░░░░░░░░░░░░░░░░░▓\t\
▓  ▓░  ░░░▒░░░  ░▓ \t█\
█░  ▓███░▒▒▒░███▓   █\
\t█░ █▓███▒▒▒███▓█  █\t\
\t▓░  ░▒▒▒▒▓▒▒▒▒░  ░▓\t\
\t\t█▒░░▒▒▒▓▓▓▒▒▒░░▒█\t\t\
\t\t\t█▒░▒█▒█▓█▒█▒░▒█\t\t\t\
\t\t\t\t█░█░█░█░█░█░█\t\t\t\t";

/*
//	--- 4:20
*/
wchar_t WEED_LEAF[627] = L"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t|.|\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t|.|\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t|\\./|\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t|\\./|\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t.\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t|\\./|\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t.\
\t\t\\^.\\\t\t\t\t\t\t\t\t\t\t|\\\\.//|\t\t\t\t\t\t\t\t\t\t/.^/\t\
\t\t\t\\--.|\\\t\t\t\t\t\t\t|\\\\.//|\t\t\t\t\t\t\t/|.--/\t\t\
\t\t\t\t\t\\--.|\t\\\t\t\t\t|\\\\.//|\t\t\t\t/\t|.--/\t\t\t\t\
\t\t\t\t\t\t\\---.|\\\t\t\t\t|\\./|\t\t\t\t/|.---/\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\\--.|\\\t\t|\\./|\t\t/|.--/\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\\..\\\t\t|.|\t\t/../\t\t\t\t\t\t\t\t\t\t\
\t\t\t_~-_^_^_^_-..\\ \\\\.//./..-_^_^_^_-~_\t\
\t\t\t\t-:-/_/_/- ^_^/|,|\\^_^ -\\_\\_\\-:-\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t/_\t/\t|\t\\\t_\\\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

wchar_t WEED_BANNER[799] = L"\t\t⣠⣤⣤⣤⡙⠦⡄\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⢀⡴⢋⣠⣤⣤⣄⡀\t\
⢠⡟⠉\t\t⠉⢻⣆⠘⣆\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⣠⠋⣰⡟⠉\t\t⠈⢻⡆\
⠸⣆\t\t⢠\t\t⣿⡆\t\t\t\t\t\t\t\t\t\t⣼⡀\t\t\t\t\t\t\t⠃\t⣿\t\t⡄\t\t⢠⡟\
\t⠙⠒⠒⠋\t\t⣽⡇⢠\t\t\t\t\t\t\t\t⣼⣿⣇\t\t\t\t\t\t\t⡄⠘⣿\t\t⠈⠒⠒⠋\t\
\t\t\t\t\t\t⢀⣿\t⡌\t\t⡀\t\t\t\t⢰⣿⣿⣿⠄\t\t\t\t\t\t⢤\t⣿⡄\t\t\t\t\t\t\
\t\t\t\t\t⢀⣾⠃⣰⠇\t\t⠹⣷⣦⣄\t⠸⣿⣿⣿\t\t⣀⣠⣤⡖⠁⠘⣇⠘⣷⡀\t\t\t\t\t\
\t\t\t\t⢠⡾⢃⣴⠋\t\t\t\t⠹⣿⣿⣿⣦⣿⣿⣿⣴⣾⣿⣿⠏\t\t\t⠙⣧⡈⢷⡄\t\t\t\t\
\t\t\t⢰⠏⢠⠞⠁\t\t\t\t\t⢀⣈⣻⣿⣿⣿⣿⣿⣿⣿⣛⠁\t\t\t\t\t⠈⠳⡄⠹⣆\t\t\t\
\t\t⡆⢨⡀⠃\t\t\t\t\t⠐⠚⠿⠿⠿⣿⣿⣿⣿⣿⡿⠿⡿⠿⠶⢄\t\t\t\t\t⠘\t⡝⢠\t\t\
\t\t⠻⢤⡹\t⣀⣤⣄⣀⡀\t\t\t\t\t⠟⠉⢉⡏⠙⠻⠄\t\t\t\t⣀⣤⣴⣶⣦⡄⠈⡠⠞\t\t\
\t\t\t\t\t⣸⣿⣿⣿⣿⡎⠑⢦⣀\t\t\t\t⠸⠇\t\t\t⢀⣠⡴⠋⢀⣿⣿⣿⣿⣿\t\t\t\t\t\
\t\t\t\t\t⠹⣿⣿⣿⡿⠋\t\t⠉⠻⠒⠤⣀\t\t⣀⠔⠚⠟⠁\t\t⠈⠻⣿⣿⣿⠟\t\t\t\t\t\
\t\t\t\t\t\t⠈⠙⠻⢧⣀⡀\t\t\t⢀⣀⣤⠷⠋\t\t\t\t⣀⣀⣀⡠⠬⠛⠉\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t⠉⣛⡵⠾⠛⠉\t\t\t\t\t\t⢠⡴⢟⠁\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t⢀⡤⠊⠁\t\t\t\t\t\t⣀⣠⠴⠚⠉\t\t⠙⠢⣀\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t⢀⣠⡶⠋\t\t\t⢀⣀⡤⠶⠛⠉⠈⠛⠶⢦⣀⡀\t\t\t⠈⠳⢦⣀\t\t\t\t\t\t\
\t\t\t\t\t\t⠿⣤⠤⠄⠒⠊⠉\t\t\t\t\t\t\t\t\t\t\t\t⠉⠉⠒⠒⠤⠤⡼⠇\t\t\t\t\t";

wchar_t LIGHTER_MEDUIM[] = L" _ \t\t\t\t\t\t\t\
| |  ) \t\t\t\
| |  .(\t\t\t\
) |_(,')\t\t\
| o______|\
|       |\t\
| ||    |\t\
| ||    |\t\
| ||    |\t\
| ||    |\t\
'-------'\t";


/*
//	--- CHRISTMAS
*/
wchar_t REINDEER_LARGE[657] = L"\t\t\t\t\t\t\t\t\t\t\t|\\\t.\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t.\t/|\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t._\\\\/\t\t\t\t\t\t\t\t\t\t\t\t\t\t\\//_.\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t',\\_,\t\t\t\t\t\t\t\t\t\t,_/,'\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\\.'\t\t\t\t\t\t\t\t\t\t'./\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t_\\\\/\t\t\t\t\t\t\t\t\\//_\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t'-.\\,\t\t\t\t\t\t,/.-'\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\\\\\t\t\t\t\t//\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t_.-`\"\"\"`-._\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t,-'-'\t_\t   \t\t_\t'-'-,\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t'--\\\to\t\t\t\to\t/'-'\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t/\t\t\t\t\t\t\t\\\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t|\t\t\\_/\t\t|\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\\\t_|_\t/\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t'---'\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

wchar_t GIFT_LARGE[881] = L"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t░░░░░\t\t\t\t░▓░░░░░\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t░░░░▒▒░░░░▒▒▒▓▓▓\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t▒░░▒▒▒░▓░░░░▒▒▒░░░\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t░░░░▒▒▒▒█▓░░░▒▒▒▒░▒▓▒▒▒░░░▒▒▒▒░░\t\t\t\t\t\
\t\t░█▓▒▒▒▒▒▒▒▓▓▓▓███▓▓▒▒▒▒▓▓▒▓▓▒▒▒▒▒▒▒▓█\t\t\
\t\t░███████▒░░░░▒░░▒▒▒▒▓█░░░░░░░▒▓▓█▓▓▓▓\t\t\
\t\t░██████▒░░▓░░█▓▒▒▒▒▒▒▒▒▒▓░░░░▒█▓▓▓▓▓▓\t\t\
\t\t\t░█▓███▒▒▒▒█████████▓▓▓▓█▓░▒▒▒▓▓▓▓▓▓\t\t\t\
\t\t\t░▓▓▓▓▓░▒▒▓████▓▓███▓▓▓▓▓▓█▒▒░▓▓▓▓▓▓\t\t\t\
\t\t\t░▓▓▓▓▓░░░▓█▓▓▓▓▓▓▓█▓▓▓▓▓██░░░▓▓▓▓▓▓\t\t\t\
\t\t\t░▓▓▓▓▓▒░░▒█▓▓▓▓▓▓▓█▓▓▓▓▓█▓░░▒▓▓▓▓▓░\t\t\t\
\t\t\t░▓▓▓▓▓▒▒▒▒█▓▓▓▓▓▓▓▓▓▓▓▓▓█▒░░▓▓▓▓▓▓░\t\t\t\
\t\t\t░█████▓▒▒▒█▓▓▓▓▓▓▓▓▓▓▓▓▓█▒░░▓▓▓▓▓▓░\t\t\t\
\t\t\t\t\t\t░▒██▒▒▒███▓▓█▓▓▓▓▓▓▓▓█░▒▒███░\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t░▒███████▓▓▓▓▓▓▓▓█▒▓░░\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t▒████▓███▒░\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

wchar_t SANTA_MEDIUM[545] = L"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t⡠⠄⠐⠒⠒⠤⠤⣀\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t⡠⠊⣠⡴⠒⢀⣀⣀⣀⡀⠉⠢⡀\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t⡰⠁⣼⠿⠚⠉⠉   ⠈⠉⠒⢵⡀\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t⢀⠃⡴⠃   ⣀⣀⣀⣀⣀⡀  ⠁\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t⢀⠔⠊⢾⡇⢀⠤⠶⡉⠐   ⠐⠈⣱⣦⡢\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t⠢\t\t⢀⠿⣅⠘ ⣴⡂ ⡀⡀⢴⣆ ⢣⡥\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t⠁⠒⠉⣄⠖⠸⠁ ⡠⡂⡀ ⣪  ⠊⢇\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t⣠⢈ ⠘⡀ ⠒⠁ ⡨⢉ ⠈⠂⠄⠺⢈⢆\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t⡝    ⠂ ⠒⢞⠻⢛⡑⠂ ⠐⠁ ⢹⠁\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t⣀⡏       ⠁    ⠄   ⠹⡀\t\t\t\t\t\t\t\t\t\t\t\t\
\t⣾⡓⢿⠆ ⡄            ⢠⠠⠷⢄\t\t\t\t\t\t\t\t\t\t\t\
\t⠈⢿⣌⢿⣄⣃           ⡀⣸⠟⢠⡾\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t⠙⢷⣛⢿⣷⣬⣦⡠⣄    ⢄⣰⣿⣿⡾⠋\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t⠙⠳⢮⣍⣉⠛⠿⢿⣮⡢⢮⣻⡿⠟⠋\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

wchar_t SNOWFLAKE_MEDIUM[] = L"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t███\t\t\t\t███\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t████\t\t\t\t████\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t███\t\t██\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t██\t█\t\t█████\t█\t██\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t████████████████████\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t██\t█\t\t████\t\t█\t██\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t███\t\t███\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t████\t\t\t\t████\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t███\t\t\t\t███\t\t\t\t\t\t\t\t\t\t\t\t";

wchar_t CHRISTMAS_TREE[755] = L"\t\t\t\t\t\t\t\t\t'.'.'\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t-=\to\t=-\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t/\t\\\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t.'.\to'.\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t/\t6\ts\t^.\\\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t/.-.o\t*.-.\\\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t`/.\t'.'9\t\t\\`\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t.'6.\t*\t\ts\to\t'.\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t/.--.s\t.6\t.--.\\\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t`/\ts\t'.\t.'\t*\t.\\`\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t.'\to\t6\t.`\t.^\t6\ts'.\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t/.---.\t*\t^\to\t.----.\\\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t`/s\t*\t`.^\ts.'\t^\t*\t\\`\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t.'\to\t,\t6\t`.'\t^\to\t\t6\t'.\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
/,-^--,\t\to\t^\t*\ts\t,----,\\\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
`'-._s.;-,_6_^,-;._o.-'\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t|\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t`\"\"\"`\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";




/*
//	--- Canadian's Day
*/
wchar_t MAPLELEAF_MEDIUM[351] = L"\t\t\t\t\t\t\t\t\t\t\t⣶⣄\t\t⢀\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t⢸⣿⣿⣷⣴⣿⡄\t\t\t\t\t⢀⡀\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t⠰⣶⣾⣿⣿⣿⣿⣿⡇\t⢠⣷⣤⣶⣿⡇\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t⠙⣿⣿⣿⣿⣿⣿⣿⣀⣿⣿⣿⣿⣿⣧⣀\t\t\t\t\t\
\t\t\t\t\t\t\t⣷⣦⣀⠘⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠃\t\t\t\t\t\
\t\t\t\t⢲⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠁\t\t\t\t\t\t\
\t\t\t\t\t⠙⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡟⠁\t\t\t\t\t\t\t\
\t\t\t\t\t⠚⠻⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠿⠿⠂\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t⠉⠙⢻⣿⣿⡿⠛⠉⡇\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t⠘⠋⠁\t\t\t⠸⡄\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⢳⡀\t\t\t\t\t\t\t\t\t\t";

wchar_t BEAVER_MEDIUM[451] = L"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t⣀⣀⣀⣀⣀\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t⣠⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣶\t⣠⣄⣼⣿⠻⢿⣷⡄\t\t\
\t\t\t\t\t⢀⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣦⣨⣿⣿⣿⣶⣾⡏⠰⠆\t\
\t\t\t\t\t⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠟⠛⠋\t\t\t\
\t\t\t\t⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠋⢻⣿⣿⣿⣿⣿⠟\t\t\t\t\t\t\t\
\t\t\t\t⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⠟⢀⣾⡿⠿⣿⣿⡇\t\t\t\t\t\t\t\t\
\t\t\t⢠⣤⡈⠛⢿⣿⣿⣿⣿⣿⣥⣀⡉⠁\t\t⠘⢿⣧⡀\t\t\t\t\t\t\t\
\t\t⢀⣾⣿⣿⣦⡀⠙⠛⠿⠿⢿⣿⣿⣿⣿⡶\t\t⠈⠻⢿⣿⠆\t\t\t\t\t\
\t\t⣼⣿⣿⣿⣿⣿⣄\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t⢠⣿⣿⣿⣿⣿⣿⣿⡄\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t⠸⣿⣿⣿⣿⣿⣿⣿⡇\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t⠙⢿⣿⣿⣿⣿⡿⠁\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t⠈⠉⠉⠁\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

wchar_t VIOLA_DESMOND[630] = L"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t⠄⠐\t⠠⠈⠄⡐\t⠄⢊⠂⠁⠢⡲⡄⠄⡀⡀\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t⢀⢡⢰⣱⡨⡢⡘⢎⢦⡹⡬⣦⣣⢅⢅⠑⢌⢂⠐⢄\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t⢨⡰⣵⣻⡳⣻⡚⣞⣮⣕⢗⢝⠺⡿⣿⣷⣯⡾⡊⡮⡐⡈\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t⠠⣱⢯⡷⢕⢨⠠⠪⠰⠱⠿⡫⡱⢌⠮⡻⣟⣞⢎⢪⣺⣪⠂⡀⠂⠁\t\t\t\t\
\t\t\t\t\t\t\t⠐⢜⠻⣣⠃⠄ ⠐  ⠂ ⡈ ⠁⠁⡙⠜⡺⣾⣷⣿⡡⡀\t\t\t\t\t\t\
\t\t\t\t\t\t\t⢠⠡⡈⡢⢁ ⠂ ⠂ ⠂⠁  ⡈ ⡀⠄⠨⢽⢿⠗⢅⢂⡀⡀⠄\t\t\t\
\t\t\t\t\t\t⢀⢇⡅⢱⠈ ⡀⠐ ⠐ ⠠ ⠐  ⠄⠠⠐⠨⣪⣷⡫⠰⡸⡽⡄\t\t\t\t\
\t\t\t\t\t\t⢸⣗⣿⡆⠅⢂⢐⠐⡇⡆⠄⠂⢀⢂⡦⡥⠠⢀⠂⢁⢑⣿⢎⣵⣟⣼⠁\t\t\t\t\
\t\t\t\t\t\t\t⣿⣿⡏ ⠘⡊⢻⢃⠳  ⢀⢳⠕⡶⠵⡢⠂ ⡐⠍⢴⣿⣟⣿\t\t\t\t\t\
\t\t\t\t\t\t\t⢽⣿⣇⠈  ⢁⠠   ⡀ ⡉⠍⠁⠂ ⠂⠠⠈⣸⣿⣿⡟\t\t\t\t\t\
\t\t\t\t\t\t\t⠰⣿⣿⡦⠈  ⠄⠠ ⢄ ⠄⠠ ⡈⢀⠁⡈⣔⣸⣿⣿⣿⡇\t\t\t\t\t\
\t\t\t\t\t\t\t⠈⢿⣿⣿⡀ ⡁⡀⡀⠁⠁ ⠄⢐ ⡐ ⠠ ⣾⣿⣿⣿⡿\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t⠙⣿⣿⣖ ⢀⠑⠭⢿⡫⠹⠈⢀⠁⡀⠐ ⠂⠽⣿⣿⠯\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t⠙⠿⣿⣷⡀  ⠁⠈ ⠂ ⠄⠔⠈ ⠁⠂⠉⠋\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t⠉⠙⠿⠧⠂⠡⠠⡁⢆⢇⠃⡁⠐⠈\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⡐ ⠁⠄⠠ ⠅⠂⠁\t\t\t\t\t\t\t\t\t\t\t\t\t";

wchar_t LOUIS_RIEL[667] = L"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⠐⣢⣤⣀\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⣀⡴⠶⣿⣿⣾⣿⣟⢦⣄⣠⣤⣄\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t⢀⣴⣶⣿⣿⣷⣦⣬⠄⡟⣿⣿⣿⣿⣻⣷⣶⣷\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t⢀⣤⣴⣿⣿⣿⣽⣿⣿⣿⣿⣶⣷⡿⣿⢿⣿⣿⣯⣿⡿\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t⣠⣿⣿⣿⡟⠛⠛⠻⠿⠿⠿⠿⠿⠿⠿⠿⢿⣿⣿⣿⣝⢳⣤⡀\t\t\t\t\t\t\
\t\t\t\t\t⢀⣴⣿⡿⣿⡏             ⠹⣿⣿⡿⣷⣿⣿⣦\t\t\t\t\t\
\t\t\t\t⣴⣏⣿⣿⣿⣿               ⣹⣿⣷⣬⣿⣿⣿⡀\t\t\t\t\
\t\t\t⢠⣿⣿⣿⣿⣿⡟ ⣀⣀⣀⣀⣀  ⢀⣀⣀⣀⣀   ⠙⣿⣿⣿⣿⣿⣿⣷⢀\t\t\
\t\t\t\t⣿⣿⣿⣿⣿⠃⢀⣭⢤⣾⣿⣿  ⠘⣿⣿⡦⢤⣤⡄  ⢹⣿⣿⣿⣿⣿⣿⣮\t\t\
\t\t\t\t⢿⣿⣿⣿⣿     ⠈⠉   ⠈⠓⠂    ⠈⡏⣿⣿⣿⣿⠏⠯\t\t\t\
\t\t\t\t⠸⣿⣿⣿⣿⡇               ⢰⣿⣿⣿⣿⣿⡷⠇\t\t\t\t\
\t\t\t\t\t⠻⢿⣿⣯⡻    ⢞⣽⣷⣤⣤⣀⠳     ⢋⣼⣿⣿⡿⠟\t\t\t\t\t\
\t\t\t\t\t\t\t⢿⣿⣿⡀ ⣰⣾⣿⣿⣿⣿⣿⣿⣷⣄    ⣾⣿⣿⠟⠁\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t⠉⠛⠛⢲⠿⠿⠋⠴⠶⠶⠶ ⠙⠿⠛⠃⢰⡶⠟⠉⠁\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t⠈⣆⡀      ⢀⣀⣤⣄  ⣀\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t⠿⣍⡛⠛⠻⠟⠛⠛⠋⡀ ⠉  ⣹⡇\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t⣾  ⠈⠛⠛⠻⢿⣋⠉  ⣀⡤⠚⠁ ⠁⠒ ⣀\t\t\t\t\t";


/*
//	--- St Paddy's Day
*/
wchar_t LEPERCHAUN[560] = L"\t\t\t\t\t\t\t\t\t\t\t⣀⣤⣤⣄⣀⣀\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t⢀⣴⡶⠟⠛⠉⠉⠉⠉⠉⢻⡿\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t⠘⣿⡆       ⢸⡇\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t⢸⣧ ⢀⣤⢤⠤⢤⣄⣘⣿⡀\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t⠈⣿⠤⠼⡄⢿⣿⡇⣯⠭⢽⣧⡀\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t⣿⠖⠚⠻⠾⠟⠒⢋⣀⡀⢻⣷⡀\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t⢀⣠⣴⡿⠶⠶⠒⢒⡛⠋⠉⠙⢛⠋⠛⠛⠿⣶⡀\t\t\t\t\t\t\t\t\
\t\t\t⢰⣾⠟⠉ ⣀⣠⠶⣶⣻⠋⢙⣿⠿⣿⣻⡷⣦⣄⣿⡇\t\t\t\t\t\t\t\t\
\t\t\t⠘⣿⣄⣴⠛⢩⠇⢸⡿⡟⠃ ⢽⡛⠓⠛⢳⣈⡿⢻⡇\t\t\t\t\t\t\t\t\
\t\t\t\t⠘⣿⣻⣧⠘⣧⠈ ⠉  ⡀⣁⣠ ⢸⢿⣷⡿⠁\t\t\t\t\t\t\t\t\
\t\t\t\t\t⠙⠿⣿ ⡇⠐⣶ ⠈⡶⠇⢰⠇⢀⣾⢿⡍\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t⣿⣀⡳⣄⡀⠑⠶⠴⠞⢁⣠⠾⢣⣤⡟\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t⢀⣴⡿⠋⠙⣇⣠⠉⠛⠒⠲⠞⠛⠁⢲⡾⢿⣶⡀\t\t\t\t\t\t\t\t\t\
\t\t\t⣰⡿⠋ ⡄ ⠈⢹⣤⢤⡀⢰⣦⣰⡖⠛⠃⣆⠙⢿⣧⡀\t\t\t\t\t\t\t\
\t⢀⣼⡟ ⢀⣾⡇⢠⡶⠋⠁⢸⠉⠹⣏⠉⢛⣶⠆⠸⣧⣀⠹⣷⣄\t\t\t\t\t\t\
\t⣼⠟ ⢀⠎⣼  ⠉⠓⢦⡼  ⠹⣤⣎⡁  ⣷⣉⡷⠈⠻⣷⡄\t\t\t\t\
⠘⣿⣄ ⠙⢦⣿  ⢠⣴⠚⡇   ⢻⣼⠁  ⢸⡿⠁\t\t⣸⡇\t\t\t\t";

wchar_t POT_O_GOLD[547] = L"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t__ .--. _\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t.-;.-\"-.-;`_;-,\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t.(_( `)-;___),-;_),\t\t\t\t\t\
\t\t\t\t\t\t\t(.( `\\.-._)-.(   ). )\t\t\t\t\
\t\t\t\t\t,(_`'--;.__\\  _).;--'`_)\t\t\t\
\t\t\t\t// )`--..__ ``` _( o )'(';. \
\t\t\t\t\\;'\t\t\t\t\t\t\t\t`````\t\t`\\\\   '.\\\\\
\t\t\t\t/\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t':.___//\
\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t'---'|\
\t\t\t;\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t;\
\t\t\t\t\\\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t/\t\
\t\t\t\t\t'.\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t.'\t\t\
\t\t\t\t\t\t\t'-,.__\t\t\t\t\t\t\t\t\t__.,-'\t\t\t\t\
\t\t\t\t\t\t\t\t(___/`````````\\___)\t\t\t\t\t";

wchar_t SHAMROCK_1[739] = L"\t\t\t\t\t\t\t\t\t\t\t\t▓▓▓▓▒▓▓▓\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t▓▓▓▓▓▓▓▓▓▓▓\t\t\t▓▒▓▓▓▓▓\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓█\t\t\
\t\t\t\t\t\t\t\t\t\t▓█▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\t\t\
\t\t\t\t\t\t\t\t\t\t\t▓█▓▓▓▓▓▓▓▓▓▓▓▓▓█▓▓▓▓██▓\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t▓█▓▓█▓▓▓▓▓▓▓█▓▓▓▓▓▓█▓\t\t\t\
\t\t▓▒▓▓▓▓▓▓▓▓\t\t█▓█▓█▓▓██▓▓▓▓▓██▓\t\t\t\t\t\
▓▒▓▓▓▓▓▓▓▓▓▓▓▓▓\t█▓▓██████▓▓\t\t\t\t\t\t\t\t\t\
▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓██▓█\t\t█▓▓▓▓▓▓▓\t\t\t\t\
▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓█\t▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓█\t\
\t▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\t▓██▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓█\
\t\t▓▓▓▓▓▓▓▓▓▓▓▓██\t▓██▓▓█▓▓▓▓▓▓▓▓▓▓▓▓▓\
\t▓▓▓▓▓▓▓▓▓▓▓███\t▓███▓▓▓▓▓▓▓▓▓▓▓▓▓█▓█\
\t\t▓██▓▓▓▓▓██▓█\t▓▓\t██▓▓▓▓▓▓▓▓█████▓█\t\
\t\t\t█▓█████▓█\t\t▓▓\t\t██▓▓▓▓▓▓▓▓▓▓█\t\t\t\t\t\
\t\t\t\t\t\t███\t\t\t▓▓▓\t\t\t█▓█▓▓▓▓▓▓▓██\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t█▓▓▓█\t\t\t\t\t█▓████▓▓██\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t█▓█\t\t\t\t\t\t\t\t\t\t\t████\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

wchar_t SHAMROCK_2[739] = L"\t\t\t\t\t\t\t\t\t\t\t▒▒▒▒\t▓▓▓▓▓▓▒\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t▓▓▓▓▓▓▓▓▓▓▓▓▓▓\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒\t▓▓▓▓▓▓▓▓\t\t\t\
\t\t\t▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\t\t\
\t▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\t\t\
▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\t\t\
▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\t\
\t▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒\
▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\t\
▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒\t\t\
\t▓▓▓▓▓▓▓▓▓▓▓\t▓\t▓▓▓▓▓▓▓▓▓▓▓▓▓\t\t\t\t\t\t\t\t\
\t\t\t▒▓▓▓▓\t\t\t\t▓\t▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t▓\t\t▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t▓\t\t▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t▓\t\t\t▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t▓\t\t\t\t▓▓▓▓▓▓▓▓▓▓▓▒\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t▓\t\t\t\t\t\t▓▓▓▓▒\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

wchar_t ST_PATS_HAT[449] = L"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t⣀⣤⣴⣶⣶⣾⣷⣶⣦⢠⣄⡀\t\t⢀⣠⡖⢲⢰⠉⣳⡀\
\t\t\t\t\t\t\t\t⢠⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⡈⣿⣿⡆\t⠻⣄⡈⠘⢃⡤⠤⠗\
\t\t\t\t\t\t\t\t⢸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇⣿⣿⡇\t⣏⡉⠉⣴ ⡉⠑⡄\
\t\t\t\t\t\t\t\t⠸⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡇⣿⣿⡇⢀⡼⣦⠜⢸⣀⠟⠋\t\
\t\t\t\t\t\t\t\t\t⣿⣿⣿⣿⣿⣿⣿⠿⠿⠿⠿⣿⠿⢷⠏\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t⢠⣭⣭⣭⣭⣭⡇⣶⣶⣶⣶⡀⣷⣿⣿⡇\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t⢸⣿⣿⣿⣿⣿⡇⠻⢿⣿⣿⡇⣿⣿⣿⣇⣀⣀⡀\t\t\t\t\t\
\t\t⢀⡠⠒⣉⣭⣭⣭⣉⡛⠻⠿⣿⡇⢸⣿⣿⣿⡇⠟⢋⣡⣤⣤⣤⣬⣅⡒⢤\t\t\
\t\t⣯⠶⠛⠉⠉⠉  ⠈⠉⠒⠢⠤⠬⠭⠭⠕⠒⠉⠁    ⠈⠉⠛⠛\t\t";


/*
//	--- Nine Eleven
*/
wchar_t NINE_11[1341] = L"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t░░░\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t░░░░░░▒▒▒░░\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t░░░\t\t░░░\t\t\t\t\t\t░░░░░░░░░▒▒\t░\t\t░░▒░░░\t\t\t\t\t\t\t\t\t\t\t\t\t▓░\t\t\t\t\t\t\t\t\t\
\t\t\t\t░░░░░░░░░░░░░░░░░░░░░░░░▒▒▒▒▒░░░░░▒▓▓▓▓░░░░\t\t\t\t\t\t\t\t\t▓░\t\t\t\t\t\t\t\t\t\
\t\t\t\t░░░░░░░░░░▒░░░░░░░░░░░░░▒▓▓▒▒▒▒░░▒▒▓▒▒░▒▓▒▒▒▒░\t\t\t\t\t\t▓░\t\t\t\t\t\t\t\t\t\
\t\t\t░░░░░░░░░░░░░   ░░ ░░░░░░▒▒▓▒▓▓▓░░▒▓▓░▒▓▓▓▒▓▓█▓▒▒░░░\t▓░\t\t\t\t\t\t\t\t\t\
\t\t░░          ░░░░░░░░░░░░▒▒▒▒▒▒▒▓░░░▒▒▓▒▒░▒░░▒▓██▓▒▒▒▒▒▓▒░░░\t\t\t\t\t\t\
\t░░░░░      ░░░░▒░ ░░░░░▒▒▒▒▓▓▒▒▒░▒▒▒▒▓▒▒▒▒▓▒▓▓▓▓▒▓▒▓▓▓▓▓▓▒▓▓▓░░\t\t\t\
\t░░░░░    ░░░░░░░░░░░░░▒▒▒░░░░▒▒░▒▒▒▓▒▓▓▒▒▓▓▒▒▒▓▓▒▒▓████▓▒░▒▒░▓░\t\t\t\
\t\t\t░░░  ░░░░░░░░░ ░░░▒▒▒▒▒░░▒░▒▒▓▓▓▓██▓▒▒▓▒▓▓███████▓▓████▓▓▓▓▓█░\t\t\
\t\t\t\t░▒░    ░░░░▒▒░░░░░▒▒▒▒▒▒▒▒▒▒▓▓▓▒▒▒▒▒▓▓█▓▓▒▒▓▓▓▓█▓▒▓▓██████▓░\t\t\t\
\t\t░▒▒░░░ ░░░░░░░░░░░░░░▒▒▒▓▒▒▒▒▒▓▓▒▒▒▒▒▓███▓▓▒▒▓▓▓▓▓▓▓▓▓██████▓░░\t\t\
\t\t\t░░░░   ░░░░░░░▒▒▒░░░▒▓▓▓▓█▓▓██▓▓▓▒▒▓▓▓██▓▓▓▓▒▓▓█▓▓██████████░\t\t\t\
\t░▒░░░░░░░░░░▒▒▒▒▒▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓██▓▓▓▓█▓▓▓▓▓▓███▒▓█████▓▓██░\t\t\t\
\t░░▒░░░░░░░░░▒▒▒▓▓▓▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░▒▒░░░▒███▒▓██▓▓▓████░\t\t\t\
\t\t\t░░░░░░▒▒▒▒▒▒▓▓▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒\t\t\t\t\t\t\t\t▒▓██▒▓█████████░\t\t\t\
\t\t\t\t░░░░░░░▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒░\t\t\t\t\t\t\t\t\t▒▓██▓▓█████▓▓█▓░\t\t\t\
\t\t\t\t\t\t\t░░░▒▒▒▒▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▒░░\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t▒▓▓▓▓▓██████▓█░\t\t\t";

wchar_t BUSH_LARGE[1274] = L"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⢀⣠⠤⢤⡐⠖⠒⠲⠶⠒⣷⡲⡖⠤⣀\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⣠⠔⣁⣀⣤⢤⠄⣉⣥⠄⠑⠺⢶⣬⣚⣦⣨⣇⡀\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⣠⡾⢋⣦⠦⠞⠉⠉⠉⠉ ⠁    ⠈⠉⢻⡿⣟⢵⠲⠤⣄\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⢠⣾⣷⣥⡿⠁           ⢄⡀  ⢿⣷⡷⣌⡢⡈⢣\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⣿⡿⣿⡿             ⣤⠘  ⡞⢿⣾⠛⢿⠘⠳⢧\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⢠⣯⣾⣿⠁              ⠈ ⠄⢸⢶⣯⡙⣶⣍⠉⠉⢆\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⢴⣾⠿⣿⣿          ⢰⣶⣶⣶⣴⣦⢀ ⢻⣬⡟⢏⠚⠳⣄⣀⣸⡄\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⢰⣿⡟⡩⢿⠄ ⢀⣴⣾⣷⣵⣄ ⠠⣴⣻⣿⣯⣯⣝⠯⢧⡈⢻⣿⣾⣮⡐⡌⢸⡬⡇\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⠘⢿⡝⡰⢠⡇⣼⠿⣫⣽⡿⢟⡻ ⢬⣖⠉⠁⠋⠋⠉⠉⠋⠁ ⠻⢿⣿⣿⣿⠽⣻⡄\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⢼⢀⢱⢾⡇⠏⡜⠋⢉⠡   ⠙⢧⡃         ⢈⡿⡏⣠⠟⡏⢡\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⠘⣿⣿⣾⡗⠊        ⢀⣙⡆     ⠠ ⢀⣐⡟⠻⡷⠂⢴⠘\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⠘⣟⣻⣱     ⠠⠿⠿⠷⠟⠉⠋        ⡸⡟⢤⡇ ⣼⠇\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⢻⣘⣽⡆                  ⠈⢁⣧⠘⠤⠊⡄\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⠈⡍⣿⣵     ⣀⣤⣶⣶⣿⣷⣶⣤⣀  ⢰⡕ ⣾⣋⣧⣠⡎\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⠹⠬⣿⡇   ⣠⠟⠋⠉⠉    ⠉⠃ ⣼⡇⢸⣿⣹⡿⣽\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⠑⣀⣷⣨⠆ ⠁         ⢀⢼⡿ ⣾⣿⣿⣷⠿\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⠈⢧⠻⡤         ⢀⣤⣿⠃⣠⣿⣿⡟⠁\t⠐\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⠑⢽⣶⣄⡀⣀⣀⣀⣤⣶⣶⣿⣿⢋⣼⣿⣿⠋  ⢀⣤⣾⣿⣦\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t⠙⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣿⣿⠟⠁ ⣀⣴⣿⣿⣿⣿⣿⣷";


/*
//	--- Valentine's
*/
wchar_t VHEART_1[529] = L"\t\t\t\t\t\t_____\t\t\t\t\t\t\t\t\t\t\t_____\t\t\t\t\t\t\
\t\t,ad8PPPP88b,\t\t\t\t\t,d88PPPP8ba,\t\t\
\td8P\"      \"Y8b,\t,d8P\"      \"Y8b\t\
dP'           \"8a8\"           `Yd\
8(              \"              )8\
I8                             8I\
\tYb,                         ,dP\t\
\t\t\"8a,                     ,a8\"\t\t\
\t\t\t\t\"8a,                 ,a8\"\t\t\t\t\
\t\t\t\t\t\t\"Yba             adP\"\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t`Y8a         a8P'\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t`88,     ,88'\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\"8b   d8\"\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\"8b d8\"\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t`888'\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";

wchar_t VHEART_2[529] = L"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t.o.o.\t\t\t\t\t\t\t.o.o.\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t.dq.o.dq\t\t\t\t\tdq.o.dq.\t\t\t\t\t\t\t\
\t\t.doq\t\t\t\t\t\t\tdq\tdq\t\t\t\t\t\t\tqod.\t\t\t\t\
\t.dq\t\t\t\t\t\t\t\t\t\t'o'\t\t\t\t\t\t\t\t\t\tdq.\t\t\t\
.dq\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tdq.\t\t\
.dq\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tdq.\t\t\
.dq\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tdq.\t\t\
\t.dq\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tdq.\t\t\t\
\t\t.dq\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tdq.\t\t\t\t\
\t\t\t.dq\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tdq.\t\t\t\t\t\
\t\t\t\t\t.dq\t\t\t\t\t\t\t\t\t\t\t\t\t\t\tdq.\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t.dq\t\t\t\t\t\t\t\t\t\t\tdq.\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t.dq\t\t\t\t\t\t\tdq.\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t.dq\t\t\tdq.\t\t\t\t\t\t\t\t\t\t\t\t\t\
\t\t\t\t\t\t\t\t\t\t\t\t\t\t'o'\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
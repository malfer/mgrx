#define NUM_LANGUAGES 3
 
#define _(sid) ((char *)GrI18nGetString(sid))

// english strings
char *eng_menubar[3] = {
    "&File",
    "&Language",
    "&Help"};
char *eng_menu1[4] = {
    "Load picture &1",
    "Load picture &2",
    "Load picture &3",
    "E&xit"};
char *eng_menu2[2] = {
    "&Hello GrGUI",
    "&About this test"};
char *eng_menu3[3] = {
    "&English",
    "&Spanish",
    "&Greek"};
char *eng_hello[5] = {
    "Hello GrGUI",
    "Welcome to MGRX and GrGUI",
    "MGRX is a small C 2D graphics library",
    "and GrGUI a miniGUI on top of MGRX",
    "visit mgrx.fgrim.com for more info"};
char *eng_about[4] = {
    "About",
    "This is the GrGUI programmer's guide",
    "test number 12, a minimal example",
    "ready to manage user window resizes"};
char *eng_generic[1] = {
    "Okey"};

// spanish strings
char *spa_menubar[3] = {
    "&Fichero",
    "&Idioma",
    "&Ayuda"};
char *spa_menu1[4] = {
    "Cargar imagen &1",
    "Cargar imagen &2",
    "Cargar imagen &3",
    "&Salir"};
char *spa_menu2[2] = {
    "&Hola GrGUI",
    "&Acerca de"};
char *spa_menu3[3] = {
    "&Inglés",
    "&Español",
    "&Griego"};
char *spa_hello[5] = {
    "Hola GrGUI",
    "Bienvenido a MGRX y a GrGUI",
    "MGRX es una librería en C de gráficos 2D",
    "y GrGUI es un miniGUI para MGRX",
    "visita mgrx.fgrim.com para más información"};
char *spa_about[4] = {
    "Acerca de",
    "Este es el programa de ejemplo número 12",
    "de la guía del programador de GrGUI",
    "muestra como manejar la redimensión de ventanas"};
char *spa_generic[1] = {
    "Vale"};

// greek strings
char *gre_menubar[3] = {
    "&Αρχείο",
    "&Γλώσσα",
    "&Βοήθεια"};
char *gre_menu1[4] = {
    "Εικόνα &1",
    "Εικόνα &2",
    "Εικόνα &3",
    "&Εξοδος"};
char *gre_menu2[2] = {
    "&Γειά σου GrGUI",
    "&Σχετικά"};
char *gre_menu3[3] = {
    "&Αγγλικά",
    "&Ισπανικά",
    "&Ελληνικά"};
char *gre_hello[5] = {
    "Γειά σου GrGUI",
    "Καλωσήλθατε στην MGRX και GrGUI",
    "Η MGRX είναι μία μικρή βιβλιοθήκη γραφικών 2Δ σε C",
    "και η GrGUI μία διεπαφή χρήστη πάνω στην MGRX",
    "Επισκεφτείτε το mgrx.fgrim.com για περισσότερες πληροφορίες"};
char *gre_about[4] = {
    "Σχετικά",
    "Αυτό είναι το παράδειγμα 12 που θα βρείτε",
    "στις οδηγίες προγραμματισμού της GrGUI",
    "με δυνατότητα αλλαγής μεγέθους του παραθύρου"};
char *gre_generic[1] = {
    "Εντάξει"};

enum SID {
    SID_OK = 0,         // generic strings

    SID_MBAR1,          // menu bar
    SID_MBAR2,
    SID_MBAR3,

    SID_MENU1_1,        // menu 1
    SID_MENU1_2,
    SID_MENU1_3,
    SID_MENU1_4,

    SID_MENU2_1,        // menu 2
    SID_MENU2_2,

    SID_MENU3_1,        // menu 3
    SID_MENU3_2,
    SID_MENU3_3,

    SID_DLGHELLOT,      // dlg hello
    SID_DLGHELLO1,
    SID_DLGHELLO2,
    SID_DLGHELLO3,
    SID_DLGHELLO4,

    SID_DLGABOUTT,      // dlg about
    SID_DLGABOUT1,
    SID_DLGABOUT2,
    SID_DLGABOUT3,

    SID_LAST
};

void setup_i18n(void)
{
    if (!GrI18nInit(NUM_LANGUAGES, SID_LAST, "undef")) {
        fprintf(stderr, "error initing GrI18n");
        exit(1);
    }

    GrI18nSetLabel(0, "English");
    GrI18nSetLabel(1, "Español");
    GrI18nSetLabel(2, "Greek");

    GrI18nAddStrings(0, SID_OK, 1, (void **)eng_generic);
    GrI18nAddStrings(0, SID_MBAR1, 3, (void **)eng_menubar);
    GrI18nAddStrings(0, SID_MENU1_1, 4, (void **)eng_menu1);
    GrI18nAddStrings(0, SID_MENU2_1, 2, (void **)eng_menu2);
    GrI18nAddStrings(0, SID_MENU3_1, 3, (void **)eng_menu3);
    GrI18nAddStrings(0, SID_DLGHELLOT, 5, (void **)eng_hello);
    GrI18nAddStrings(0, SID_DLGABOUTT, 4, (void **)eng_about);

    GrI18nAddStrings(1, SID_OK, 1, (void **)spa_generic);
    GrI18nAddStrings(1, SID_MBAR1, 3, (void **)spa_menubar);
    GrI18nAddStrings(1, SID_MENU1_1, 4, (void **)spa_menu1);
    GrI18nAddStrings(1, SID_MENU2_1, 2, (void **)spa_menu2);
    GrI18nAddStrings(1, SID_MENU3_1, 3, (void **)spa_menu3);
    GrI18nAddStrings(1, SID_DLGHELLOT, 5, (void **)spa_hello);
    GrI18nAddStrings(1, SID_DLGABOUTT, 4, (void **)spa_about);

    GrI18nAddStrings(2, SID_OK, 1, (void **)gre_generic);
    GrI18nAddStrings(2, SID_MBAR1, 3, (void **)gre_menubar);
    GrI18nAddStrings(2, SID_MENU1_1, 4, (void **)gre_menu1);
    GrI18nAddStrings(2, SID_MENU2_1, 2, (void **)gre_menu2);
    GrI18nAddStrings(2, SID_MENU3_1, 3, (void **)gre_menu3);
    GrI18nAddStrings(2, SID_DLGHELLOT, 5, (void **)gre_hello);
    GrI18nAddStrings(2, SID_DLGABOUTT, 4, (void **)gre_about);
}

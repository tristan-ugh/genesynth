#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>

// Nous mockons l'enregistrement de l'API table de rocprofiler pour WSL2.
// Cette fonction retourne normalement un code d'erreur (0 = succès).
int rocprofiler_register_library_api_table(const char* name, void* api_table) {
    // On ignore silencieusement l'enregistrement pour éviter la vérification KFD.
    return 0; 
}

// Mock également de la fonction d'association de table
int rocprofiler_set_api_table(const char* name, uint64_t lib_version, void* api_table) {
    return 0;
}

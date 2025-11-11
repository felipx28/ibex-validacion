#include "ibex.h"
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip> // Para std::setprecision
#include <cmath>   // Para std::isnan, std::isinf

using namespace std;
using namespace ibex;

const double CLAMP_VALUE = 1e7;
const double UMBRAL_CERO = 1e-7;
const double MAX_VALUE = 9223372036854775807;

int main(int argc, char* argv[]) {

    if (argc < 2) {
        cerr << "Error: Se necesita el nombre del archivo del problema como argumento." << endl;
        cerr << "Uso: ./foo <nombre_archivo.bch>" << endl;
        return 1;
    }

    char* filename = argv[1];

    try {
        // Se carga el sistema usando el constructor que lee un archivo.
        // Ver en ibex_System.h: System(const char* filename, ...);
        System sys(filename);

        // Se usa una referencia a la caja inicial para mayor claridad.
        // Ver en ibex_System.h: IntervalVector box;
        const IntervalVector& initial_box = sys.box;

        // -----------------------------------------------------------------
        // 2. Calcular las 7 características requeridas de la CAJA INICIAL
        // -----------------------------------------------------------------
        
        // Característica 1: Número de variables
        // Ver en ibex_System.h: const int nb_var;
        int num_vars = sys.nb_var;

        // Característica 2: Número de restricciones
        // Ver en ibex_System.h: const int nb_ctr;
        int num_ctrs = sys.nb_ctr;

        // Característica 3 y 4: Límites de la función objetivo
        double lb_f_obj = 0.0;
        double ub_f_obj = 0.0;
        
        // Se comprueba si la función objetivo existe.
        // Ver en ibex_System.h: Function* goal;
        if (sys.goal) {
            // Se evalúa la función objetivo sobre la caja inicial.
            // Ver en ibex_System.h: Interval goal_eval(const IntervalVector& box) const;
            Interval f_obj_interval = sys.goal->eval(initial_box);
            lb_f_obj = f_obj_interval.lb();
            ub_f_obj = f_obj_interval.ub();
        }
        
        // Las siguientes características se calculan sobre sys.box (un IntervalVector).
        // Estas funciones (sum_diam, max_diam, min_diam) son métodos de la clase IntervalVector.

        // Característica 5: Suma de todos los diámetros (search_space)
        double search_space = 0;
        
        for (int i = 0; i < initial_box.size(); ++i) {
            // Sumar los diámetros de cada intervalo en la caja.
            search_space += initial_box[i].diam();
        }

        // Característica 6: Diámetro más grande (bigger_diam)
        double bigger_diam = initial_box.max_diam();

        // Característica 7: Diámetro más pequeño (lower_diam)
        double lower_diam = initial_box.min_diam();
        
        // Limpieza de valores (NaN, Inf) y manejo de límites para evitar problemas en Python
        if (std::abs(lb_f_obj) < UMBRAL_CERO) lb_f_obj = 0.0;
        if (std::abs(ub_f_obj) < UMBRAL_CERO) ub_f_obj = 0.0;
        if (std::abs(search_space) < UMBRAL_CERO) search_space = 0.0;
        if (std::abs(bigger_diam) < UMBRAL_CERO) bigger_diam = 0.0;
        if (std::abs(lower_diam) < UMBRAL_CERO) lower_diam = 0.0;

        if (lb_f_obj >= CLAMP_VALUE) lb_f_obj = MAX_VALUE;
        else if (lb_f_obj <= -CLAMP_VALUE) lb_f_obj = -MAX_VALUE;

        if (ub_f_obj >= CLAMP_VALUE) ub_f_obj = MAX_VALUE;
        else if (ub_f_obj <= -CLAMP_VALUE) ub_f_obj = -MAX_VALUE;

        if (std::isnan(lb_f_obj) || std::isinf(lb_f_obj)) lb_f_obj = 0.0;
        if (std::isnan(ub_f_obj) || std::isinf(ub_f_obj)) ub_f_obj = 0.0;
        if (std::isnan(search_space) || std::isinf(search_space)) search_space = 0.0;
        if (std::isnan(bigger_diam) || std::isinf(bigger_diam)) bigger_diam = 0.0;
        if (std::isnan(lower_diam) || std::isinf(lower_diam)) lower_diam = 0.0;

        // -----------------------------------------------------------------
        // 3. Escribir las características en el archivo de salida
        // -----------------------------------------------------------------
        
        ofstream outFile("temp.txt");
        if (!outFile) {
            cerr << "Error: No se pudo abrir el archivo de salida temp.txt" << endl;
            return 1;
        }
        
        // Escribir en el formato exacto requerido por el script de Python: "((f1 ; f2 ; ...))"
        outFile << std::fixed << std::setprecision(17);
        outFile << "(("
                << (double)num_vars     << " ; "
                << (double)num_ctrs     << " ; "
                << lb_f_obj             << " ; "
                << ub_f_obj             << " ; "
                << search_space         << " ; "
                << bigger_diam          << " ; "
                << lower_diam
                << "))";
        
        outFile.close();

    } catch (ibex::Exception& e) {
        cerr << "Ha ocurrido un error en Ibex: " << e.what() << endl;
        return 1;
    } catch (std::exception& e) {
        cerr << "Ha ocurrido un error inesperado: " << e.what() << endl;
        return 1;
    }

    return 0;
}
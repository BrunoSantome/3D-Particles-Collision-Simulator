#include <iostream>
#include <random>
#include <fstream>
#include <iomanip>
#include <vector>
#include <omp.h>
#define G 6.674e-11
const unsigned int numThreads =4;
using namespace std;
using std::fixed;
using std::setprecision;
double media = pow(10,21);
double desviacion = pow(10,15);

//Creamos un struct de arrays para Objeto ponemos el valor 4000
// porque es el maximo que vamos a utilizar si quisiesemos usar mas deberiamos aumentarlo
struct Objeto{
    vector <double> x;
    vector <double> y;
    vector <double>z;
    vector <double> m;

};
//CReamos un struct de arrays para velociades
struct Velocidad{
    vector <double> x;
    vector <double> y;
    vector <double> z;
};
//CReamos un struct de arrays para FUerzas
struct Fuerza{
    vector <double> x;
    vector <double> y;
    vector <double> z;
};
//CReamos un struct de arrays para Aceleraciones
struct Aceleracion{
    vector <double> x;
    vector <double> y;
    vector <double> z;
};

//inicializa los struct the vectores
void inicializarArrays(Objeto &obj, int num_objects,Velocidad &velocidad, Fuerza &fuerza, Aceleracion &aceleracion) {

    for (int i = 0; i < num_objects; i++) {

        obj.x.push_back(0);
        obj.y.push_back(0);
        obj.z.push_back(0);
        obj.m.push_back(0);

        velocidad.x.push_back(0);
        velocidad.y.push_back(0);
        velocidad.z.push_back(0);

        fuerza.x.push_back(0);
        fuerza.y.push_back(0);
        fuerza.z.push_back(0);

        aceleracion.x.push_back(0);
        aceleracion.y.push_back(0);
        aceleracion.z.push_back(0);
    }
}

//Usamos La funcion CreateOBjects para crear todos los objetos dentro de los limites del espacio y a partir de la semilla aleatoria
void CreateObjects(double num_obj, double seed, double tam_recinto, Objeto &obj){
    std::mt19937_64 gen (seed)  ;
    std::uniform_real_distribution<> dis(0.0, tam_recinto);
    std::normal_distribution<> ndis(media, desviacion);
    //Vamos generando los objetos y metiendolos direcatament el varios array de objetos
    for (int i =0; i<num_obj ; i++){
        obj.x[i]=dis(gen);
        obj.y[i]=dis(gen);
        obj.z[i]=dis(gen);
        obj.m[i]=ndis(gen);

    }
}
//La funciÃ³n InitialFIleoutput nos saca el archivo de la configuracion inicial de coordenadas, masa y velocidades
void initialFileOutput(double num_objects, double tam_recinto,  Objeto objetos, Velocidad velocidades, double time_step){
    std::ofstream myfile;
    myfile.open("init_config.txt");
    myfile <<fixed<<setprecision(3)<< tam_recinto <<  " " << time_step << " " << num_objects << "\n";
    for (int i=0; i<num_objects;i++){
        myfile << fixed << setprecision(3) << objetos.x[i] << " " << objetos.y[i] << " " << objetos.z[i] << " "<< velocidades.x[i]<< " "<< velocidades.y[i] << " "<< velocidades.z[i]<< " " << objetos.m[i] << "\n" ;
    }
    myfile.close();
}

//Aqui escribimos en el archivo final el resultado de la operación
void finalFileOutput(double num_objects, double tam_recinto,  Objeto objetos, Velocidad velocidades, double time_step){
    std::ofstream myfile;
    myfile.open("final_config.txt");
    myfile <<fixed<<setprecision(3)<< tam_recinto <<  " " << time_step << " " << num_objects << "\n";
    for (int i=0; i<num_objects;i++){
        myfile <<fixed<<setprecision(3)<< objetos.x[i] << " " << objetos.y[i] << " " << objetos.z[i] << " "<< velocidades.x[i]<< " "<< velocidades.y[i] << " "<< velocidades.z[i]<< " " << objetos.m[i] << "\n" ;
    }
    myfile.close();
}

//Esta función se encarga de calcular las fuerzas, aceleraciones, velocidad y posiciones
void CalcularFuerzas( int num_objects, Objeto &objetos, Velocidad &velocidades, Fuerza &fuerzas, Aceleracion &aceleraciones, double time_step, double size_enclosure) {

    //Creamos un struct auxiliar de arrays tipo double

    //Doble bucle para evaluar todas las posibles combinaciones de fuerzas entre los objetos
#pragma omp parallel for num_threads(numThreads)
    for (int i = 0; i < num_objects; i++) {
        //a cada iteracion, la aceleracion se vuelve 0, para que no sea acumulativa

//NOs encargamos en este segundo bucle de el calculo de las fuerzas y por tanto de su sumatorio

        for (int j = i + 1; j < num_objects; j++) {


	   // Calculamos distancias
            double x = objetos.x[j] - objetos.x[i];
            double y = objetos.y[j] - objetos.y[i];
            double z = objetos.z[j] - objetos.z[i];
            
            //distancia euclediana
            double dis = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
            double div = pow(dis, 2);
            double mul = (G * objetos.m[i] * objetos.m[j]) / div;
            //Calculo de el vector Fuerza


            //Calculo del sumatorio de fuerzas
            fuerzas.x[i] = fuerzas.x[i] + mul * (objetos.x[j] - objetos.x[i]) / dis;
            fuerzas.y[i] = fuerzas.y[i] + mul * (objetos.y[j] - objetos.y[i]) / dis;
            fuerzas.z[i] = fuerzas.z[i] + mul * (objetos.z[j] - objetos.z[i]) / dis;

            fuerzas.x[j] = fuerzas.x[j] - mul * (objetos.x[j] - objetos.x[i]) / dis;
            fuerzas.y[j] = fuerzas.y[j] - mul * (objetos.y[j] - objetos.y[i]) / dis;
            fuerzas.z[j] = fuerzas.z[j] - mul * (objetos.z[j] - objetos.z[i]) / dis;



        }
    }
    
    //En este segundo bucle se calculan las nuevas posiciones y velocidades en cada iteración
#pragma omp parallel for num_threads(numThreads)
    for (int i = 0; i < num_objects; i++) {
        //Calculo de la aceleración
        aceleraciones.x[i] = fuerzas.x[i] / objetos.m[i];
        aceleraciones.y[i] = fuerzas.y[i] / objetos.m[i];
        aceleraciones.z[i] = fuerzas.z[i] / objetos.m[i];
        //Calculo de la velocidad en funcion de la velocidad anterior + la aceleracion por el tiempo.
        velocidades.x[i] = velocidades.x[i] + aceleraciones.x[i] * time_step;
        velocidades.y[i] = velocidades.y[i] + aceleraciones.y[i] * time_step;
        velocidades.z[i] = velocidades.z[i] + aceleraciones.z[i] * time_step;
        //Igualamos las posiciones del objeto auxiliar las posiciones de nuestro struct objetos principal


        //Calculamos el nuevo vector posicion
        objetos.x[i] = objetos.x[i] + (velocidades.x[i] * time_step);
        objetos.y[i] = objetos.y[i] + (velocidades.y[i] * time_step);
        objetos.z[i] = objetos.z[i] + (velocidades.z[i] * time_step);


        //Las condiciones que tienen las coordendas.
        //para que las posiciones no sean negativas se igualan a 0 y se cambia la direccion de la velocidad

        if (objetos.x[i] <= 0) {
            objetos.x[i] = 0.0;
            velocidades.x[i] = 0.0 - velocidades.x[i];
        }
        if (objetos.y[i] <= 0) {
            objetos.y[i] = 0.0;
            velocidades.y[i] = 0.0 - velocidades.y[i];
        }
        if (objetos.z[i] <= 0) {
            objetos.z[i] = 0.0;
            velocidades.z[i] = 0.0 - velocidades.z[i];
        }
        //para que las coordenadas no sean mayores que el tamaño del  recinto las igualamos al tamaño y cambiamos la direccion de la velocidad
        if (objetos.x[i] >= size_enclosure) {
            objetos.x[i] = size_enclosure;
            velocidades.x[i] = 0.0 - velocidades.x[i];
        }
        if (objetos.y[i] >= size_enclosure) {
            objetos.y[i] = size_enclosure;
            velocidades.y[i] = 0.0 - velocidades.y[i];
        }
        if (objetos.z[i] >= size_enclosure) {
            objetos.z[i] = size_enclosure;
            velocidades.z[i] = 0.0 - velocidades.z[i];
        }
        fuerzas.x[i] = 0;
        fuerzas.y[i] = 0;
        fuerzas.z[i] = 0;
    }

    //Volvemos a meter las nuevas posiciones en nuestro struct principal a partir del auxiliar


}
//La funcion eliminaroBjeto es llamada por la funcion colision, cada vez que se produce una.
void EliminarObject (Objeto &objetos, Velocidad &velocidades, int &num_objects, int &j){

    //Como su nombre indica, el nuevo objeto fruto de los otros dos remplaza al objeto b
    for (int i = j; i<(num_objects-1);i++ ){
        //se adelantan en 1 las posiciones del array
        objetos.x[i] = objetos.x[i+1];
        objetos.y[i] = objetos.y[i+1];
        objetos.z[i] = objetos.z[i+1];
        objetos.m[i] = objetos.m[i+1];
        //seadelantan en 1 las posiciones de la velociadad
        velocidades.x[i] = velocidades.x[i+1];
        velocidades.y[i] = velocidades.y[i+1];
        velocidades.z[i] = velocidades.z[i+1];
    }

    //se disminuye el numero de objetos
    num_objects-=1;
    j--;
}
//La funcion colision, Se ocupa de comprobar entre todas las combinaciones de objetos, si ocurren colisiones
void colision(Objeto &objetos, Velocidad &velocidades, int &num_objects) {

//paralelizamos
#pragma omp parallel for num_threads(numThreads)
    for (int i = 0; i < num_objects; i++) {
        for (int j = 0; j < num_objects; j++) {
            //Comrpobamos que las coordenadas de las dos particulas es menos a la unidad
            double x = fabs(objetos.x[i] - objetos.x[j]);
            double y = fabs((objetos.y[i] - objetos.y[j]));
            double z = fabs((objetos.z[i] - objetos.z[j]));
            if ((x < 1) && (y < 1) && (z < 1) &&
                (i != j)) {
                //sumamos masas y velocidades de los dos objetos colisionados
                objetos.m[i] = objetos.m[i] + objetos.m[j];
                velocidades.x[i] = velocidades.x[i] + velocidades.x[j];
                velocidades.y[i] = velocidades.y[i] + velocidades.y[j];
                velocidades.z[i] = velocidades.z[i] + velocidades.z[j];
                //Llamada a la funcion para eliminar el objeto en la posicion j
                EliminarObject(objetos, velocidades, num_objects, j);

            }
        }

    }

}

int main(int argc, char *argv[]) {

    //pasamos los argumentos que le pasamos al programa y los convertimos en doubles
    int num_objects = atof(argv[1]);
    double num_iterations = atof(argv[2]);
    double seed = atof(argv[3]);
    double size_enclosure = atof(argv[4]);
    double time_step = atof(argv[5]);
    //comprobación que hay suficientes argumentos, salida de error estandar.
    if (argc - 1 != 5) {
        fprintf(stderr, "Error: Program invoked with %i parameters, insert 5 paramaters in total please. \n", argc - 1);
        exit(-1);
    }
    //COmprobacion que los argumentos no sean negativos o igual a 0, (los caracteres se transforman en 0)
    if (argc - 1 == 5) {
        cout << "Program invoked with " << argc - 1 << " parameters." << endl;
        //salida estandar de error
        if (num_objects <= 0) {

            fprintf(stdout, "Error: invalid box size in num_objects = %i, only positive numbers allowed ", num_objects);
            exit(-2);
        }

        if (num_iterations <= 0) {
            fprintf(stdout, "Error: invalid box size in num_iterations %f, only positive numbers allowed ",
                    num_iterations);
            exit(-2);
        }

        if (seed <= 0) {
            fprintf(stdout, "Error: invalid box size in seed = %f, only positive numbers allowed ", seed);
            exit(-2);
        }

        if (size_enclosure <= 0) {
            fprintf(stdout, "Error: invalid box size in size_enclosure = %f, only positive numbers allowed ",
                    size_enclosure);
            exit(-2);
        }
        if (time_step <= 0) {
            fprintf(stdout, "Error: invalid box size in time_step = %f, only positive numbers allowed ", time_step);
            exit(-2);
        }

        cout << "\n Arguments: " << endl;
        cout << " num_objects: " << num_objects << endl;
        cout << " num_iterations: " << num_iterations << endl;
        cout << " random_seed: " << seed << endl;
        cout << " size_enclosure: " << size_enclosure << endl;
        cout << " time_step: " << time_step << endl;

    }

        //creamos nuestro struct objeto de arrays  principal. aqui se van a almacenar nuestros objetos
        Objeto objetos;
        //creamos el struct de velociades
        Velocidad velocidades;
        //struct fuerzas
        Fuerza fuerzas;
        //struct Aceleraciones
        Aceleracion aceleraciones;
        //inicializamos a cero los arrays de las posiciones
        inicializarArrays(objetos,num_objects,velocidades,fuerzas,aceleraciones);


        //llamada a funcion, creamos los objetos iniciales con el numero de objetos el tamaño del cubo y la semilla, junto al array
        CreateObjects(num_objects, seed, size_enclosure, objetos);
        //el archivo con la configuración inicial
        initialFileOutput(num_objects, size_enclosure, objetos, velocidades, time_step);
        //se comprueba al principio si existen colisiones entre los objetos creados aleatoriamente
        colision(objetos, velocidades, num_objects);
        //Bucle que genera la simulacion, este bucle se repite el numero de iteraciones que le pasamos como argumento
        for (int i = 0; i < num_iterations; i++) {
            //en cada iteración llamamos a la funcion que calcula todos los vectores
            CalcularFuerzas(num_objects, objetos, velocidades, fuerzas, aceleraciones, time_step, size_enclosure);
            //en cada iteracion, con los vectores creados y actualizados comprobamos si hay colisiones entre
            colision(objetos, velocidades, num_objects);

        }
        //creamos el archivo de simulacion final llamando a su correspondiente funcion
        finalFileOutput(num_objects, size_enclosure, objetos, velocidades, time_step);

    }



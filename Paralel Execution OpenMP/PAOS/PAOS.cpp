#include <stdio.h>
#include <iostream>
#include <random>
#include <fstream>
#include <iomanip>
#include <omp.h>
#define G 6.674e-11
const unsigned int numThreads =8;
using namespace std;
//Para fijar una cierta precision de decimales
using std::fixed;
using std::setprecision;
double media = pow(10,21);
double desviacion = pow(10,15);

//Inicializamos el struct de OBjetos
struct Objeto{
    double x =0;
    double y =0;
    double z =0;
    double m =0;
};
//inicilizamos el struct de Velocidad
struct Velocidad{
    double x = 0;
    double y = 0;
    double z = 0;
};
//inicilizamos el struct de Fuerza
struct Fuerza{

    double x = 0;
    double y = 0;
    double z = 0;
};
//inicilizamos el struct de Aceleracion
struct Aceleracion{
    double x = 0;
    double y = 0;
    double z = 0;
};
//Usamos La funcion CreateOBjects para crear todos los objetos dentro de los limites del espacio y a partir de la semilla aleatoria
void CreateObjects(int num_obj, int seed, double tam_recinto, Objeto *obj) {
    std::mt19937_64 gen(seed);
    std::uniform_real_distribution<> dis(0.0, tam_recinto);
    std::normal_distribution<> d{media, desviacion};
    //Vamos generando los objetos y metiendolos direcatament el un array de objetos
    for (int i = 0; i < num_obj; i++) {
        obj[i].x = dis(gen);
        obj[i].y = dis(gen);
        obj[i].z = dis(gen);
        obj[i].m = d(gen);
    }
}
//La función InitialFIleoutput nos saca el archivo de la configuracion inicial de coordenadas, masa y velocidades
void initialFileOutput(int num_objects, double tam_recinto,  Objeto objetos[], Velocidad velocidades[], double time_step){
    std::ofstream myfile;
    myfile.open("init_config.txt");
    myfile <<fixed << setprecision(3) << tam_recinto <<  " " << time_step << " " << num_objects << "\n";
    for (int i=0; i<num_objects;i++){
        myfile << fixed << setprecision(3) << objetos[i].x << " " << objetos[i].y << " " << objetos[i].z << " "<< velocidades[i].x<< " "<< velocidades[i].y << " "<< velocidades[i].z<< " " << objetos[i].m << "\n" ;
    }
    myfile.close();
}


//La función FInalFIleOutput nos saca el archivo de la configuracion final de coordenadas, masa y velocidades
void finalFileOutput(int num_objects, double tam_recinto,  Objeto *objetos, Velocidad *velocidades, double time_step){
    std::ofstream myfile;
    myfile.open("final_config.txt");
    myfile <<fixed << setprecision(3) << tam_recinto <<  " " << time_step << " " << num_objects << "\n";
    for (int i=0; i<num_objects;i++){
        myfile << objetos[i].x << " " << objetos[i].y << " " << objetos[i].z << " "<< velocidades[i].x<< " "<< velocidades[i].y << " "<< velocidades[i].z<< " " << objetos[i].m << "\n" ;
    }
    myfile.close();
}
//Esta función se encarga de calcular las fuerzas, aceleraciones, velocidad y posiciones
void CalcularFuerzas(int num_objects, Objeto *objetos, Velocidad *velocidades, Fuerza *fuerzas,Aceleracion *aceleraciones, double time_step, double size_enclosure){

    //Doble bucle para evaluar todas las posibles combinaciones de fuerzas entre los objetos
#pragma omp parallel for num_threads(numThreads)
    for (int i =0; i < num_objects; i++) {
       
        //a cada iteracion, la aceleracion se vuelve 0, para que no sea acumulativa
        //NOs encargamos en este segundo bucle de el calculo de las fuerzas y por tanto de su sumatorio
        for (int j = i + 1; j < num_objects; j++) {

	   //calculamos las distancias 
            double x = objetos[j].x - objetos[i].x;
            double y = objetos[j].y - objetos[i].y;
            double z =objetos[j].z - objetos[i].z ;
            //distancia euclediana y vector fuerza
            double dis = sqrt(pow(x,2)+pow(y,2)+pow(z,2));
            double div = pow(dis, 2);
            double mul = (G * objetos[i].m * objetos[j].m) / div;
            //Calculo de el vector Fuerza

            //Calculo del sumatorio de fuerzas
            fuerzas[i].x = fuerzas[i].x + mul * (objetos[j].x - objetos[i].x) / dis;
            fuerzas[i].y = fuerzas[i].y + mul * (objetos[j].y - objetos[i].y) / dis;
            fuerzas[i].z = fuerzas[i].z + mul * (objetos[j].z - objetos[i].z) / dis;

            fuerzas[j].x = fuerzas[j].x - mul * (objetos[j].x - objetos[i].x) / dis;
            fuerzas[j].y = fuerzas[j].y - mul * (objetos[j].y - objetos[i].y) / dis;
            fuerzas[j].z = fuerzas[j].z - mul * (objetos[j].z - objetos[i].z) / dis;

        }

    }
    //aqui paralelizamos el programa

#pragma omp parallel for num_threads(numThreads)
    for (int i=0; i < num_objects; i++){

        //Calculo de la aceleración
        aceleraciones[i].x = fuerzas[i].x/objetos[i].m;
        aceleraciones[i].y = fuerzas[i].y/objetos[i].m;
        aceleraciones[i].z = fuerzas[i].z/objetos[i].m;
        //Calculo de la velocidad en funcion de la velocidad anterior + la aceleracion por el tiempo.
        velocidades[i].x = velocidades[i].x + aceleraciones[i].x * time_step;
        velocidades[i].y = velocidades[i].y + aceleraciones[i].y * time_step;
        velocidades[i].z = velocidades[i].z + aceleraciones[i].z * time_step;


        //Calculamos el nuevo vector posicion
        objetos[i].x = objetos[i].x + (velocidades[i].x * time_step);
        objetos[i].y = objetos[i].y + (velocidades[i].y * time_step);
        objetos[i].z = objetos[i].z + (velocidades[i].z * time_step);

        //Las condiciones que tienen las coordendas.
        //para que las posiciones no sean negativas se igualan a 0 y se cambia la direccion de la velocidad
        if (objetos[i].x <=0){
            objetos[i].x = 0.0;
            velocidades[i].x = 0.0 - velocidades[i].x;
        }
        if (objetos[i].y <=0){
            objetos[i].y = 0.0;
            velocidades[i].y = 0.0 - velocidades[i].y;
        }
        if (objetos[i].z <=0){
            objetos[i].z = 0.0;
            velocidades[i].z = 0.0 - velocidades[i].z;
        }

        //para que las coordenadas no sean mayores que el tamaño del  recinto las igualamos al tamaño y cambiamos la direccion de la velocidad
        if (objetos[i].x >= size_enclosure){
            objetos[i].x = size_enclosure;
            velocidades[i].x = 0.0 - velocidades[i].x;
        }
        if (objetos[i].y >= size_enclosure){
            objetos[i].y = size_enclosure;
            velocidades[i].y = 0.0 - velocidades[i].y;
        }
        if (objetos[i].z >= size_enclosure){
            objetos[i].z = size_enclosure;
            velocidades[i].z = 0.0 - velocidades[i].z;
        }
        fuerzas[i].x = 0;
        fuerzas[i].y = 0;
        fuerzas[i].z = 0;

    }
}
//La funcion eliminaroBjeto es llamada por la funcion colision, cada vez que se produce una.

void EliminarObject (Objeto *objetos, Velocidad *velocidades, int &num_objects, int &j){

    //Como su nombre indica, el nuevo objeto fruto de los otros dos remplaza al objeto b
    for (int i = j; i<(num_objects-1);i++ ){
        //se adelantan en 1 las posiciones del array
        objetos[i].x = objetos[i+1].x;
        objetos[i].y = objetos[i+1].y;
        objetos[i].z = objetos[i+1].z;
        objetos[i].m = objetos[i+1].m;
        //seadelantan en 1 las posiciones de la velociadad
        velocidades[i].x = velocidades[i+1].x;
        velocidades[i].y = velocidades[i+1].y;
        velocidades[i].z = velocidades[i+1].z;
    }


    //se disminuye el numero de objetos
    num_objects-=1;
    j--;
}

//La funcion colision, Se ocupa de comprobar entre todas las combinaciones de objetos, si ocurren colisiones
void colision(Objeto *objetos, Velocidad *velocidades, int &num_objects) {

//aqui paralelizamos el programa
#pragma omp parallel for num_threads(numThreads)
    for (int i = 0; i < num_objects; i++) {
        for (int j = i+1; j < num_objects; j++) {
            //Comrpobamos que las coordenadas de las dos particulas es menos a la unidad
            double x = fabs(objetos[i].x - objetos[j].x);
            double y = fabs((objetos[i].y - objetos[j].y));
            double z = fabs((objetos[i].z - objetos[j].z));
            if ((x < 1) && (y < 1) && (z < 1) ) {

                //sumamos masas y velocidades de los dos objetos colisionados
                objetos[i].m = objetos[i].m + objetos[j].m;
                velocidades[i].x = velocidades[i].x + velocidades[j].x;
                velocidades[i].y = velocidades[i].y + velocidades[j].y;
                velocidades[i].z = velocidades[i].z + velocidades[j].z;
                //Llamada a la funcion para eliminar el objeto en la posicion j
                EliminarObject(objetos, velocidades, num_objects, j);

            }
        }
    }

}
int main(int argc, char *argv[]) {

    //pasamos los argumentos que le pasamos al programa y los convertimos en doubles
    int num_objects = atof(argv[1]) ;
    double num_iterations = atof(argv[2]);
    double seed = atof(argv[3]);
    double size_enclosure = atof(argv[4]);
    double time_step = atof (argv[5]);

    //comprobación que hay suficientes argumentos, salida de error estandar.
    if (argc - 1 != 5) {
        fprintf(stderr, "Error: Program invoked with %i parameters, insert 5 paramaters in total please. \n", argc-1);
        exit(-1);
    }
    //COmprobacion que los argumentos no sean negativos o igual a 0, (los caracteres se transforman en 0)

    //COmprobacion que los argumentos no sean negativos o igual a 0, (los caracteres se transforman en 0)
    if (argc - 1 == 5) {
        cout << "Program invoked with " << argc - 1 << " parameters." << endl;
        //salida estandar de error
        if (num_objects <= 0) {

            fprintf(stdout, "Error: invalid box size in num_objects = %i, only positive numbers allowed ",num_objects );
            exit(-2);
        }

        if (num_iterations <= 0) {
            fprintf(stdout, "Error: invalid box size in num_iterations %f, only positive numbers allowed ",num_iterations );
            exit(-2);
        }

        if (seed <= 0) {
            fprintf(stdout, "Error: invalid box size in seed = %f, only positive numbers allowed ",seed);
            exit(-2);
        }

        if (size_enclosure <= 0) {
            fprintf(stdout, "Error: invalid box size in size_enclosure = %f, only positive numbers allowed ", size_enclosure );
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
  //  int num_objects=(int)num_objects1;
    //creamos nuestro array de structs objeto principal. aqui se van a almacenar nuestros objetos
    Objeto *objetos = new Objeto[(int) num_objects];
    //creamos el array de velociades
    Velocidad *velocidades = new Velocidad[(int) num_objects];

    Fuerza *fuerzas = new Fuerza[(int) num_objects];

    Aceleracion *aceleraciones = new Aceleracion[(int) num_objects];
    //llamada a funcion, creamos los objetos iniciales con el numero de objetos el tamaño del cubo y la semilla, junto al array
    CreateObjects(num_objects,seed,size_enclosure,objetos);
    //el archivo con la configuración inicial
    initialFileOutput(num_objects,size_enclosure, objetos, velocidades, time_step);
    //se comprueba al principio si existen colisiones entre los objetos creados aleatoriamente
    colision(objetos,velocidades,num_objects);

    //Bucle que genera la simulacion, este bucle se repite el numero de iteraciones que le pasamos como argumento
    for(int i =0;i < num_iterations; i++){

        //en cada iteración llamamos a la funcion que calcula todos los vectores
        CalcularFuerzas(num_objects,objetos,velocidades,fuerzas,aceleraciones ,time_step, size_enclosure);

        //en cada iteracion, con los vectores creados y actualizados comprobamos si hay colisiones entre
        colision(objetos,velocidades,num_objects);

    }
    //creamos el archivo de simulacion final llamando a su correspondiente funcion
    finalFileOutput(num_objects,size_enclosure,objetos,velocidades,time_step);

}

#include <stdio.h>
#include <iostream>
#include <random>
#include <fstream>
#include <iomanip>
#define G 6.674e-11

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
void initialFileOutput(double num_objects, double tam_recinto,  Objeto objetos[], Velocidad velocidades[], double time_step){
    std::ofstream myfile;
    myfile.open("init_config.txt");
    myfile <<fixed << setprecision(3) << tam_recinto <<  " " << time_step << " " << num_objects << "\n";
    for (int i=0; i<num_objects;i++){
        myfile << fixed << setprecision(3) << objetos[i].x << " " << objetos[i].y << " " << objetos[i].z << " "<< velocidades[i].x<< " "<< velocidades[i].y << " "<< velocidades[i].z<< " " << objetos[i].m << "\n" ;
    }
    myfile.close();
}


//La función FInalFIleOutput nos saca el archivo de la configuracion final de coordenadas, masa y velocidades
void finalFileOutput(double num_objects, double tam_recinto,  Objeto *objetos, Velocidad *velocidades, double time_step){
    std::ofstream myfile;
    myfile.open("final_config.txt");
    myfile <<fixed << setprecision(3) << tam_recinto <<  " " << time_step << " " << num_objects << "\n";
    for (int i=0; i<num_objects;i++){
        myfile << objetos[i].x << " " << objetos[i].y << " " << objetos[i].z << " "<< velocidades[i].x<< " "<< velocidades[i].y << " "<< velocidades[i].z<< " " << objetos[i].m << "\n" ;
    }
    myfile.close();
}
//Esta función se encarga de calcular las fuerzas, aceleraciones, velocidad y posiciones
void CalcularFuerzas(double num_objects, Objeto *objetos, Velocidad *velocidades, double time_step, double size_enclosure){

    //Creamos un array auxiliar de estructuras tipo objeto
    Objeto *objetos2=new Objeto[(int)num_objects];

    //Doble bucle para evaluar todas las posibles combinaciones de fuerzas entre los objetos
    for (int i =0; i < num_objects; i++){
        //a cada iteracion, la aceleracion se vuelve 0, para que no sea acumulativa
        double aceleracionX = 0;
        double aceleracionY = 0;
        double aceleracionZ = 0;
        //NOs encargamos en este segundo bucle de el calculo de las fuerzas y por tanto de su sumatorio
        for (int j =0; j < num_objects; j++){
            if(i!=j) {

                double mul = G * objetos[i].m * objetos[j].m;

                double x = objetos[j].x - objetos[i].x;
                double y = objetos[j].y - objetos[i].y;
                double z = objetos[j].z - objetos[i].z;

                x = pow(x, 2);
                y = pow(y, 2);
                z = pow(z, 2);
                double div = x+y+z;
                div = pow(sqrt(div), 3);
                //Calculo de el vector Fuerza

                double fuerzaX = mul/div * (objetos[j].x - objetos[i].x);
                double fuerzaY = mul/div * (objetos[j].y - objetos[i].y);
                double fuerzaZ = mul/div * (objetos[j].z - objetos[i].z);
                //Calculo del sumatorio de fuerzas
                aceleracionX = aceleracionX + fuerzaX;
                aceleracionY = aceleracionY + fuerzaY;
                aceleracionZ = aceleracionZ + fuerzaZ;

            }

        }

        //Calculo de la aceleración
        aceleracionX = aceleracionX/objetos[i].m;
        aceleracionY = aceleracionY/objetos[i].m;
        aceleracionZ = aceleracionZ/objetos[i].m;
        //Calculo de la velocidad en funcion de la velocidad anterior + la aceleracion por el tiempo.
        velocidades[i].x = velocidades[i].x + aceleracionX * time_step;
        velocidades[i].y = velocidades[i].y + aceleracionY * time_step;
        velocidades[i].z = velocidades[i].z + aceleracionZ * time_step;

        //Igualamos al vector auxiliar las posiciones de nuestro array de struct objetos principal
        objetos2[i].x= objetos[i].x;
        objetos2[i].y= objetos[i].y;
        objetos2[i].z= objetos[i].z;
        objetos2[i].m= objetos[i].m;

        //Calculamos el nuevo vector posicion
        objetos2[i].x = objetos2[i].x + (velocidades[i].x * time_step);
        objetos2[i].y = objetos2[i].y + (velocidades[i].y * time_step);
        objetos2[i].z = objetos2[i].z + (velocidades[i].z * time_step);

        //Las condiciones que tienen las coordendas.
        //para que las posiciones no sean negativas se igualan a 0 y se cambia la direccion de la velocidad
        if (objetos2[i].x <=0){
            objetos2[i].x = 0.0;
            velocidades[i].x = 0.0 - velocidades[i].x;
        }
        if (objetos2[i].y <=0){
            objetos2[i].y = 0.0;
            velocidades[i].y = 0.0 - velocidades[i].y;
        }
        if (objetos2[i].z <=0){
            objetos2[i].z = 0.0;
            velocidades[i].z = 0.0 - velocidades[i].z;
        }

        //para que las coordenadas no sean mayores que el tamaño del  recinto las igualamos al tamaño y cambiamos la direccion de la velocidad
        if (objetos2[i].x >= size_enclosure){
            objetos2[i].x = size_enclosure;
            velocidades[i].x = 0.0 - velocidades[i].x;
        }
        if (objetos2[i].y >= size_enclosure){
            objetos2[i].y = size_enclosure;
            velocidades[i].y = 0.0 - velocidades[i].y;
        }
        if (objetos2[i].z >= size_enclosure){
            objetos2[i].z = size_enclosure;
            velocidades[i].z = 0.0 - velocidades[i].z;
        }

    }
    //Volvemos a meter las nuevas posiciones en nuestro array principal a partir del auxiliar
    for (int i =0; i < num_objects; i++) {
        objetos[i].x = objetos2[i].x ;
        objetos[i].y = objetos2[i].y;
        objetos[i].z =objetos2[i].z ;
        objetos[i].m = objetos2[i].m ;
    }

}

//La funcion eliminaroBjeto es llamada por la funcion colision, cada vez que se produce una.

void EliminarObject (Objeto *objetos, Velocidad *velocidades, double &num_objects, int &j){

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
void colision(Objeto *objetos, Velocidad *velocidades, double &num_objects) {



    for (int i = 0; i < num_objects; i++) {
        for (int j = 0; j < num_objects; j++) {
            //Comrpobamos que las coordenadas de las dos particulas es menos a la unidad
            double x = fabs(objetos[i].x - objetos[j].x);
            double y = fabs((objetos[i].y - objetos[j].y));
            double z = fabs((objetos[i].z - objetos[j].z));
            if ((x < 1) && (y < 1) && (z < 1) &&
                (i != j)) {
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
    double num_objects = atof(argv[1]) ;
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
    if (argc-1 == 5) {
        cout << "Program invoked with " << argc - 1 << " parameters." << endl;
        //salida estandar de error
        if (num_objects <= 0 || num_iterations <= 0 || seed <= 0 || size_enclosure <= 0 || time_step <= 0) {
            fprintf(stdout, "Error: invalid box size, only positive numbers allowed \n");
            cout << "Arguments: " << endl;
            cout << " num_objects: " << num_objects << endl;
            cout << " num_iterations: " << num_iterations << endl;
            cout << " random_seed: " << seed << endl;
            cout << " size_enclosure: " << size_enclosure << endl;
            cout << " time_step: " << time_step << endl;
            exit(-2);
        }
    }
    //creamos nuestro array de structs objeto principal. aqui se van a almacenar nuestros objetos
    Objeto *objetos = new Objeto[(int) num_objects];
    //creamos el array de velociades
    Velocidad *velocidades = new Velocidad[(int) num_objects];
    //llamada a funcion, creamos los objetos iniciales con el numero de objetos el tamaño del cubo y la semilla, junto al array
    CreateObjects(num_objects,seed,size_enclosure,objetos);
    //el archivo con la configuración inicial
    initialFileOutput(num_objects,size_enclosure, objetos, velocidades, time_step);
    //se comprueba al principio si existen colisiones entre los objetos creados aleatoriamente
    colision(objetos,velocidades,num_objects);
    //Bucle que genera la simulacion, este bucle se repite el numero de iteraciones que le pasamos como argumento
    for(int i =0;i < num_iterations; i++){

        //en cada iteración llamamos a la funcion que calcula todos los vectores
        CalcularFuerzas(num_objects,objetos,velocidades, time_step, size_enclosure);
        //en cada iteracion, con los vectores creados y actualizados comprobamos si hay colisiones entre
        colision(objetos,velocidades,num_objects);

    }
    //creamos el archivo de simulacion final llamando a su correspondiente funcion
    finalFileOutput(num_objects,size_enclosure,objetos,velocidades,time_step);

}
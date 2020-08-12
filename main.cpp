#include "common.h"
#include "IDW.h"
#include "raster.h"
#include "cost_dist.h"
int main() {
    clock_t start_global = clock();//inicia tiempo global
    std::string str;
//------------------------------------------------------------------------variables
    int M,N,M_i,N_i;//M: fila y N: columna
    int i;
    int cell_null;//valor nulo comunidad
    int cell_null_2;//valor nulo friccion
    int scale;//escala del mapa
    int num_com=0;//numero de localidades en el mapa
    float exp=1.005;//exponente IDW
    int cont=0;//numero de localidades a explorar en el ciclo while
// ------------------------------------------------------------------------matrices
    float** fric_map; //mapa friccion
    float** map_cost_dist;//mapa costo distancia
    float** map_local;//mapa con localidades (ubicacion)
// ------------------------------------------------------------------------mapas
    map<int, Raster::local> localidades;
    map<int, float> bio_local;
    Raster::tiempo time_local;
    map<int, Raster::tiempo> time_ejecucion;
//------------------------------------------------------------------------objetos
    Raster objrast;
    IDW objIDW;
    Cost_dist objcost;
//-------------------------------------------------------------------------------------------------------carga de archivos
    //mapa fricción
    fric_map = objrast.read_tif_matrix("/home/otavio/Documentos/Proyecto_OpenMP/Serial/fricc_w.tif", M, N, scale,cell_null_2);
    printf("--------No. columnas: %d\n", M);
    printf("--------No. renglones: %d\n", N);
    // mapa localidades
    map_local = objrast.read_tif_matrix("/home/otavio/Documentos/Proyecto_OpenMP/Serial/locs_10_int.tif", M_i, N_i,scale,cell_null);
    // guardamos los requisitos de las comunidades en un mapa
    objrast.carga_requisitos("/home/otavio/Documentos/Proyecto_OpenMP/Serial/fwuse.csv",bio_local);
    // guardamos las localidades en un mapa para ordenarlas
    objrast.leer_localidades(map_local,M_i,N_i,localidades,cell_null,num_com);
    printf("el total de comunidades es %d\n",num_com);
//-------------------------------------------------------------------------------------------------------valores iniciales
    Cost_dist::position datos[100];
    exp=1.005;
    float **final = new float *[M]; //matriz IDW para 1 localidad
    float **suma = new float *[M];//matriz IDW para todas las localidades
    for ( i = 0; i < M; i++){
        final[i] = new float[N];
        suma[i] = new float[N];
    }
    objIDW.reset_Matrix(final, M, N, -1.0);//llena la matriz inicial del valor indicado
    objIDW.reset_Matrix(suma, M, N, 0); //llena la matriz inicial del valor indicado
    std::map<int,float>::iterator bio; //iterados mapa requisitos localidad
    std::map<int,float>::iterator init;
    std::map<int,float>::iterator fin;
    std::map<int, Raster::local>::iterator lugar; //iterados mapa ubicacion localidades
    //-------------------------------------------------------------------------------------------------------inicia calculo modelos
    init= bio_local.begin();
    cout << "llave inicial " << init->first<<endl;
    fin = bio_local.end();
    cout << "llave final " << fin->first<<endl;
    int start=int(init->first);
    int end= int(fin->first);
    #pragma omp parallel for private(i,bio,lugar)
    for(i=start;i<=end;i++){
        bio=bio_local.find(i);
        if(bio->second!=0) { //busca localidad para la que se tengan requisitos
            lugar = localidades.find(bio->first);//buscar ubicacion de la localidad
            //-------------------------------------------------------------------------------------------------------inicia costo ditancia
            clock_t start_cost_dist = clock();//inicia tiempo cost_dist
            map_cost_dist = objcost.cost_distance(lugar->second.x, lugar->second.y, fric_map, M, N, datos);//regresa matriz costo distancia
            clock_t end_cost_dist = clock();//termina tiempo
            time_local.CD  = (float)(end_cost_dist - start_cost_dist) / (float) CLOCKS_PER_SEC;//calcula tiempo de ejecucion
            //objrast.matrix_to_tiff(map_cost_dist, M, N,cont+1,"mapa_cost_dist_");
            //-------------------------------------------------------------------------------------------------------inicia IDW
            clock_t start_IDW = clock();//inicia tiempo cost_dist
            objIDW.IDW_cost_dist(bio->second, cell_null_2, map_cost_dist, final,suma,lugar->second.x, lugar->second.y, M,N,exp);//regresa matriz IDW
            clock_t end_IDW = clock();//termina tiempo
            time_local.IDW = (float)(end_IDW - start_IDW) / (float) CLOCKS_PER_SEC;//calcula tiempo de ejecucion
            time_ejecucion.insert(pair<int, Raster::tiempo>(bio->first,time_local));//se guarda en el mapa la localidad que se exploró como llave y la estructura con los tiempos CD e IDW
            objIDW.reset_Matrix(final, M, N, -1.0);//resetear final
            cont++;//se suman las localidades calculadas
            cout<<"exploracion: "<<bio->first<<" completa"<<endl;
        }
        else{
            cout << "no entró comunidad " << bio->first<<endl;
        }
        //bio_local.erase(bio);//borrar comunidad ya explorada
        //bio= bio_local.begin();
    }
    objrast.matrix_to_tiff(suma, M, N,cont,"mapa_IDW_");//crea tiff de IDW de todas las localidades calculadas
    objrast.guardar_tiempos(time_ejecucion);
    clock_t end_global = clock();//termina tiempo
    double duration = (double)(end_global - start_global) / (double) CLOCKS_PER_SEC;//calcula tiempo de ejecucion
    printf("tiempo global: %lf segundos \n", duration);
    return 0;
}


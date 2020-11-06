#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <sys/wait.h>
#include <ctime>
#include <iomanip>

using namespace std;

int main(int argc, char** argv){

	string src_usr_name;
	string src_file_path;
	string file_name;
	string src_host;
	string src_file_dir;
	
	string targ_usr_name;
	string targ_file_dir;
	string targ_host;
	
	string src_info;
	string targ_info;
	
	string port("1234");
	
	string cmd_send;
	string cmd_recv;
	
	string cmd_src_md5;
	string cmd_targ_md5;
	
	time_t time_init;
	time_t time_end;
	
	src_info= argv[1];
	targ_info= argv[2];
	
	src_host       = src_info.substr(src_info.find('@') + 1, src_info.find(':') - src_info.find('@') - 1);
	src_usr_name   = src_info.substr(0,src_info.find('@'));
	
	src_file_path  = src_info.substr(src_info.find(':') + 1);
	src_file_dir   = src_file_path.substr(0,src_file_path.find_last_of({'/', '\\'}));
	file_name	   = src_info.substr(src_info.find_last_of({'/', '\\'}) + 1);
	
	
	targ_host      = targ_info.substr(targ_info.find('@') + 1, targ_info.find(':') - targ_info.find('@') - 1);
	targ_usr_name  = targ_info.substr(0,targ_info.find('@'));
	targ_file_dir  = targ_info.substr(targ_info.find(':') + 1);
	
	
	//Constroi o cmd de send
	cmd_send="ssh " + src_usr_name +"@"+ src_host + " \"tar -c -C "+ src_file_dir + " " + file_name + " | pigz -i9 | nc -w 1 -l " + port + " > /dev/null 2>&1 & \" ";	 
	
	//Constroi o cmd de recv
	cmd_recv ="ssh -n " + src_usr_name + "@" + src_host + " \"cd " + targ_file_dir + "; nc " + src_host + " " + port + " | pigz -d | tar xvf - \" ";
	
	//Constroi cmds de md5 do source e target
	cmd_src_md5="ssh " + src_usr_name +"@"+ src_host + " \"md5sum " + src_file_path+" \" ";
	cmd_targ_md5="ssh " + targ_usr_name +"@"+ targ_host + " \"md5sum " + targ_file_dir + "/" + file_name + " \" ";
		
	
	/*	 
	 *  Relizando MD5 no source host
	*/
	cout <<"Fazendo ssh para calcular o md5 no source host: " + src_host << endl; 
	char buffer[1000];
	FILE * f=popen(cmd_src_md5.c_str(),"r");
	fscanf(f,"%s",buffer);
	fclose(f);
	
	/*	 
	 *  Calculando tamanho do arquivo
	*/
	cout <<"Fazendo ssh para calcular o tamanho do arquivo no source host: " + src_host << endl;	
	string cmd_get_file_size="ssh "+ src_usr_name +"@"+ src_host + " \"du -sb " + src_file_path + " | awk '{print \\$1}' \" ";
	FILE * f2=popen(cmd_get_file_size.c_str(),"r");
	char buffer2[1000];
	fscanf(f2,"%s",buffer2);
	fclose(f2);	
	long long int file_size=stoll(buffer2);		
	
	/*	 
	 *  Iniciando transferencia do arquivo
	*/
	time(&time_init);
	cout <<"Fazendo ssh para iniciar a transferencia do arquivo do source host: " + src_host << endl;	
	system(cmd_send.c_str());
	
	/*	 
	 *  Iniciando recebimento do arquivo
	*/	
	cout <<"Fazendo ssh com o destino para aprovar a transferencia para o target host: " + targ_host << endl;
	system(cmd_recv.c_str());		
	time(&time_end);
		
	/*	 
	 *  Relizando MD5 no target host
	*/
	cout <<"Fazendo ssh para calcular o md5 no target host: " + targ_host << endl;
	FILE * f3=popen(cmd_targ_md5.c_str(),"r");
	char buffer3[1000];
	fscanf(f3,"%s",buffer3);
	fclose(f3);
	
	
	/*	 
	 *  Comparando o MD5
	*/
	if(strcmp(buffer,buffer3)==0){
		double send_time = difftime(time_end,time_init);
		double velo=((double)file_size/(1<<20))/send_time;
		
		cout << "Sucesso no envio do arquivo: " + file_name << endl;
		cout << fixed << setprecision(2);
		cout << "O tempo de transferencia foi de: " << (send_time<60?send_time:send_time/60.0) << (send_time < 60?" segundos":" minutos") << endl;
		cout <<  "A velocidade media de transferencia foi de: "<< velo << " MB/s" << endl;
		
	}
	else{
		cout << "Falha no envio do arquivo: " + file_name << endl;
	}		
	
	return 0;	
	

}

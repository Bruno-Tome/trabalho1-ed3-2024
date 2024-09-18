# Diretiva para compilar o código
all:
	gcc -o trabalho1 main.c 

# Diretiva para executar o código compilado
run: all
	./trabalho1

# Diretiva para limpar os arquivos compilados
clean:
	rm -f trabalho1

A implementação foi baseada no material e códigos disponíveis em : 
--> https://omaraflak.medium.com/ray-tracing-from-scratch-in-python-41670e6a96f9
-->https://github.com/OmarAflak/RayTracer-Kotlin?source=post_page-----41670e6a96f9--------------------------------

O código final desse material é escrito em Python, e percebe-se ao executá-lo que o desempenho deixa a desejar, demorando alguns segundos para renderizar uma cena simples em resolução 300x200.
A partir do estudo desse conteúdo, então foi desenvolvido código em C++, buscando uma implementação com desempenho superior.

Além disso, foi acrescentado ao código uso da ferramenta de paralelismo OpenMP, de forma a permitir uso de múltiplos núcleos do processador na renderização , e aumentando o desempenho. ( número de núcleos definido pelo argumento de execução -t)
Para garantir optimalidade, a compilação é feita utilizando a flag -O3, de otimização automática do código.

Exemplo de compilação e execução:

$ g++ -fopenmp -o ray_tracer ray_t.cpp -std=c++17 -g -O3

$ ./ray_tracer -t 4 

As imagens de teste estão todas em Full HD (1920x1080).

Teste de cenário simples com algumas esferas:
![Imagem renderizada](https://ibb.co/CJQXC6n)

Teste de Paralelismo utilizando diferentes números de núcleos e 200 esferas:
![Imagem com +200 Esferas](URL_da_Imagem)

Núcleos  Tempo (s)
1         4.47
2         3.00
4         1.71
8         1.18
12        1.04


Para efeito de comparação, tentou-se reproduzir esse último cenário com o código original em python( em FullHD ), mas cada linha de 1920 pixels demorava em torno de 3.2 s para ser renderizada, mostrando que o código implementado representa um bom ganho de desempenho a partir das mesmas ideias teóricas para implementação do Ray Tracing.

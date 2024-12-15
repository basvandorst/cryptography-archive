-----BEGIN PGP SIGNED MESSAGE-----

		     Phil's Pretty Good Software
			       presenta

				 ===
				 PGP
				 ===

			 Pretty Good Privacy
			Intimidad Bastante Buena
		 Cifrado de clave p£blica para todos


		      --------------------------
			Gu¡a del Usuario de PGP
		      Volumen I: Temas esenciales
		      ---------------------------
			 por Philip Zimmermann
		     Revisada el 14 de junio de 1993


		      PGP Versi¢n 2.3 - 13 junio 93
			  Programa escrito por
			    Philip Zimmermann
				   con
	      Branko Lankester, Hal Finney y Peter Gutmann




Sinopsis:  PGP utiliza criptograf¡a de clave p£blica para proteger el 
correo electr¢nico y los ficheros de datos. Comun¡quese con seguridad 
con personas a las que nunca ha visto, sin necesidad de canales seguros 
para intercambiar claves. PGP es r pido y tiene muchas prestaciones, 
entre las que se incluyen una completa gesti¢n de claves, firmas 
digitales, compresi¢n de datos y un buen dise¤o ergon¢mico.


Programa y documentaci¢n (c) Copyright 1990-1992 Philip Zimmermann.
Para obtener m s informaci¢n sobre licencias, distribuci¢n, copyright, 
patentes, marcas registradas, limitaciones de responsabilidad y 
controles de exportaci¢n de PGP, v‚ase la secci¢n "Consideraciones 
legales" en la "Gu¡a del usuario de PGP, volumen II: Temas especiales".

(ø indica que hay una nota del traductor en el fichero NOTAS.ES)


Indice
======

Descripci¢n general
Por qu‚ puede necesitar PGP
C¢mo funciona
Instalaci¢n de PGP
C¢mo utilizar PGP
  Para ver un resumen de las instrucciones
  Encriptaci¢n de un mensaje
  Encriptaci¢n de un mensaje para m£ltiples destinatarios
  Firma de un mensaje
  Firma y posterior encriptaci¢n
  Utilizaci¢n de encriptaci¢n convencional exclusivamente
  Gesti¢n de claves
    Generaci¢n de claves RSA
    Adici¢n de una clave al fichero
    Supresi¢n de una clave o identificador del fichero
    Extracci¢n (copia) de una clave del fichero
    Visualizaci¢n del contenido del fichero
    C¢mo proteger las claves p£blicas contra manipulaci¢n
    C¢mo controla PGP la validez de las claves
    C¢mo proteger las claves secretas contra revelaci¢n
    Revocaci¢n de una clave p£blica
    ¨Qu‚ pasa si pierde la clave secreta?
Temas avanzados
  Env¡o de texto cifrado por canales de correo-E: formato Radix-64
  Variable de entorno para la trayectoria
  Establecimiento de par metros de configuraci¢n: CONFIG.TXT
Puntos vulnerables
Cuidado con los elixires m gicos
Referencia r pida para PGP
Consideraciones legales
Agradecimientos
Acerca del autor


Descripci¢n general
===================

Pretty Good(mr) Privacy (PGP), "intimidadø bastante buena", de Phil's 
Pretty Good Software, es una aplicaci¢n inform tica de criptograf¡a de 
alta seguridad para MSDOS, Unix, VAX/VMS y otros ordenadores. PGP 
permite intercambiar ficheros y mensajes con intimidad, autenticaci¢n y 
comodidad. Intimidad quiere decir que s¢lo pueden leer el mensaje 
aquellos a quienes va dirigido. Autenticaci¢n quiere decir que los 
mensajes que parecen ser de una persona s¢lo pueden venir de esa persona 
en particular. Comodidad quiere decir que la intimidad y la 
autenticaci¢n se logran sin los problemas de gesti¢n de claves asociados 
a otros programas de criptograf¡a convencional. No se necesitan canales 
seguros para intercambiar claves entre usuarios, por lo que PGP resulta 
mucho m s f cil de utilizar. Esto se debe a que PGP est  basado en una 
potente tecnolog¡a nueva llamada criptograf¡a de "clave p£blica".

PGP combina la comodidad del criptosistema de clave p£blica de 
Rivest-Shamir-Adleman (RSA) con la velocidad de la criptograf¡a 
convencional, res£menes de mensajes para firmas digitales, compresi¢n de 
datos antes de encriptar, un buen dise¤o ergon¢mico y una completa 
gesti¢n de claves. Por otra parte, PGP realiza las funciones de clave 
p£blica m s r pido que la mayor¡a de las otras implementaciones 
inform ticas. PGP es criptograf¡a de clave p£blica para todos.

PGP no lleva incorporada comunicaci¢n por modem. Para ello debe 
utilizarse otro programa distinto.

Este documento, "Volumen I: Temas esenciales", s¢lo explica los 
conceptos esenciales sobre la utilizaci¢n de PGP, y deber¡an leerlo 
todos los usuarios. El "Volumen II: Temas especiales" trata las 
caracter¡sticas avanzadas y otros temas especiales, y est  orientado a 
aquellos usuarios que vayan a utilizar PGP m s en serio. Ninguno de los 
documentos ofrece detalles sobre la tecnolog¡a en la que se basan los 
algoritmos criptogr ficos y las estructuras de datos.


Por qu‚ puede necesitar PGP
===========================

Es personal. Es privado. Y no le concierne a nadie m s que a usted. 
Puede estar planificando una campa¤a pol¡tica, tratando sobre sus 
impuestos o teniendo una relaci¢n il¡cita. O puede estar haciendo algo 
que seg£n usted no deber¡a ser ilegal, pero que lo es. Sea lo que fuere, 
no quiere que nadie m s lea su correo electr¢nico (correo-E)ø privado ni 
sus documentos confidenciales. No hay nada malo en afirmar su derecho a 
la intimidad. El derecho a la intimidad es tan b sico como la 
Constituci¢nø.

Quiz  piense que su correo-E es lo bastante leg¡timo como para no 
necesitar encriptaci¢n. Si usted es en realidad un ciudadano respetuoso 
con la ley, sin nada que ocultar, ¨por qu‚ no env¡a siempre el correo 
habitual en postales? ¨Por qu‚ no se somete voluntariamente a pruebas de 
detecci¢n de drogas? ¨Por qu‚ exige un mandamiento judicial para que la 
polic¡a registre su casa? ¨Est  intentando esconder algo? Debe de ser un 
subversor, o un traficante de drogas, si oculta el correo en sobres. O 
quiz  un exc‚ntrico paranoico. ¨Tienen los ciudadanos que cumplen la ley 
alguna necesidad de encriptar su correo-E?

¨Qu‚ pasar¡a si todo el mundo creyese que los ciudadanos observantes de 
la ley deber¡an utilizar postales para enviar el correo?  Si alg£n 
esp¡ritu valiente intentase afirmar su intimidad utilizando un sobre, 
levantar¡a sospechas. Quiz  las autoridades abrir¡an su correo para ver 
qu‚ est  ocultando. Afortunadamente no vivimos en un mundo as¡, y todos 
protegen la mayor parte del correo con sobres. Nadie levanta sospechas 
por afirmar su intimidad con un sobre. Hay seguridad en los grandes 
n£meros. De forma an loga, ser¡a interesante que todo el mundo utilizase 
habitualmente el cifrado en el correo-E, fuese inocente o no, para que 
nadie levantase sospechas por afirmar de esa manera su derecho a la 
intimidad. Piense en ello como una forma de solidaridad.

Hoy en d¡a, si el gobierno quiere invadir la intimidad de los ciudadanos 
corrientes tiene que emplear una cierta cantidad de esfuerzo y dinero en 
interceptar y abrir al vapor el correo normal, y en escuchar, y quiz  
transcribir, las conversaciones telef¢nicas. Este tipo de control, muy 
laborioso, no resulta pr ctico a gran escala. S¢lo se realiza en casos 
importantes, donde parece que va a merecer la pena.

Cada vez una mayor parte de nuestra comunicaci¢n privada se dirige por 
canales electr¢nicos. El correo electr¢nico reemplazar  gradualmente al 
correo convencional. Los mensajes por correo-E son, sencillamente, 
demasiado f ciles de interceptar y de explorar para buscar palabras 
interesantes. Puede hacerse a gran escala, f cilmente, habitualmente, 
autom ticamente y de una forma imposible de detectar. La NSA explora ya 
de esta manera los cablegramas internacionales.

Nos dirigimos hacia un futuro en el que los pa¡ses estar n cruzados de 
lado a lado por redes de datos basadas en fibra ¢ptica de alta 
capacidad, conectando todos nuestros ordenadores personales, cada vez 
m s ubicuos. El correo-E ser  la norma para todos, no la novedad que 
resulta hoy en d¡a. Puede que el gobierno proteja nuestro correo-E con 
algoritmos de cifrado dise¤ados por ellos mismos. Y puede que la mayor¡a 
de la gente conf¡e en ellos. Pero algunas personas preferir n tomar sus 
propias medidas de protecci¢n.

La propuesta de ley 266 del Senado de los Estados Unidos, una propuesta 
conjunta contra el delito, ten¡a oculta una medida inquietante. Si esta 
resoluci¢n no vinculante hubiese llegado a ser ley, habr¡a obligado a 
los fabricantes de equipos de comunicaciones seguras a incluir 
"escotillas" en sus productos, para que el Gobierno pudiese leer 
cualquier mensaje cifrado. {Su traducci¢n al castellano} es la 
siguiente:  "Es la opini¢n del Congreso que los proveedores de servicios 
de comunicaci¢n electr¢nica y los fabricantes de equipos para servicios 
de comunicaci¢n electr¢nica deben garantizar que los sistemas de 
comunicaci¢n permitan al Gobierno obtener el contenido en texto normal 
de las comunicaciones de voz, datos y otras comunicaciones, cuando est‚n 
adecuadamente autorizados por la ley". Esta medida fue desestimada tras 
una rigurosa protesta por parte de defensores de la libertad civil y de 
grupos empresariales.

En 1992, la propuesta del FBI sobre intervenci¢n de telefon¡a digital se 
present¢ en el Congreso norteamericano. Obligar¡a a todos los 
fabricantes de equipos de comunicaciones a integrar unos puertos 
especiales para la intervenci¢n a distancia, que permitir¡a al FBI 
intervenir todo tipo de comunicaci¢n electr¢nica desde sus oficinas. 
Aunque nunca ha conseguido ning£n apoyo en el Congreso gracias a la 
oposici¢n ciudadana, se volver  a presentar en 1993.

Lo m s alarmante es la nueva en‚rgica iniciativa de la Casa Blanca sobre 
pol¡tica criptogr fica, desarrollada en la NSA durante cuatro a¤os, y 
presentada el 16 de Abril de 1993. La parte principal de esta iniciativa 
es un dispositivo criptogr fico construido por el Gobierno, llamado el 
"chip Clipper", que contiene un nuevo algoritmo criptogr fico secreto de 
la NSA. El Gobierno est  animando a las empresas privadas a que lo 
incluyan en todos sus productos de comunicaciones seguras, como 
tel‚fonos, FAX, etc. AT&T est  poniendo Clipper en todos sus productos 
seguros para voz. La trampa:  en la f brica, cada chip Clipper se 
cargar  con su propia clave £nica, y el Gobierno mantiene una copia en 
dep¢sito. Pero no hay que preocuparse-- el Gobierno promete que s¢lo 
utilizar  esas claves para leer las comunicaciones cuando est‚ 
autorizado por la ley. Naturalmente, para que Clipper sea efectivo, el 
siguiente paso l¢gico ser¡a proscribir otras formas de criptograf¡a.

Si la intimidad se proscribe, s¢lo los proscritos tendr n intimidad. Los 
servicios de inteligencia tienen acceso a tecnolog¡a criptogr fica de 
calidad. Lo mismo ocurre con los grandes traficantes de armas y de 
drogas. Tambi‚n disponen de ellos los contratistas del ej‚rcito, las 
compa¤¡as de petr¢leo y otros gigantes empresariales. Pero la mayor¡a de 
la gente normal y de las organizaciones pol¡ticas de base no han tenido 
nunca a su alcance una tecnolog¡a asequible para utilizar criptolog¡a de 
clave p£blica de "grado militar". Hasta ahora.

PGP da a la gente la capacidad de dejar su intimidad en sus propias 
manos. Hay una creciente demanda social para ello. Por eso lo escrib¡.


C¢mo funciona
=============

Ser¡a de ayuda que usted estuviese familiarizado con el concepto de 
criptograf¡a en general, y con el de criptograf¡a de clave p£blica en 
particular. En cualquier caso, he aqu¡ unas cuantas observaciones como 
introducci¢n.

En primer lugar, algo de vocabulario b sico. Supongamos que quiero 
enviarle un mensaje que nadie excepto usted pueda leer. Podr¡a 
"encriptar" o "cifrar" el mensaje, que significa revolverlo de una forma 
tremendamente complicada, haci‚ndolo ilegible para cualquiera que no sea 
usted, el destinatario original del mensaje. Yo pongo una "clave" 
criptogr fica para encriptar el mensaje, y usted tiene que utilizar la 
misma clave para descifrarlo o "desencriptarlo". Por lo menos as¡ 
funciona en los criptosistemas convencionales de "clave £nica".

En los criptosistemas convencionales, como el US Federal Data Encryption 
Standard (DES) {Norma Federal para Cifrado de Datos en EEUU}, se utiliza 
una sola clave para encriptar y desencriptar. Por lo tanto, hay que 
transmitir primero la clave por medio de un canal seguro, para que ambas 
partes la sepan antes de enviar mensajes cifrados por canales inseguros. 
Este proceso puede resultar inc¢modo. Si se tiene un canal seguro para 
intercambiar claves, ¨para qu‚ se necesita entonces criptograf¡a?

En criptosistemas de clave p£blica, todo el mundo tiene dos claves 
complementarias, una revelada p£blicamente y otra secreta. Cada clave 
abre el c¢digo que produce la otra. Saber la clave p£blica no sirve para 
deducir la clave secreta correspondiente. La clave p£blica puede 
publicarse y distribuirse ampliamente por una red de comunicaciones. 
Este protocolo proporciona intimidad sin necesidad de ese tipo de canal 
seguro que requieren los criptosistemas convencionales.

Cualquiera puede utilizar la clave p£blica de un destinatario para 
encriptar un mensaje, y ‚ emplear  su propia clave secreta 
correspondiente para desencriptarlo. S¢lo ‚l podr  hacerlo, porque nadie 
m s tiene acceso a esa clave secreta. Ni siquiera la persona que lo 
encript¢ podr¡a desencriptarlo.

Tambi‚n se proporciona autenticaci¢n para mensajes. La clave secreta del 
remitente puede emplearse para encriptar un mensaje, "firm ndolo". As¡ 
se crea una firma digital, que el destinatario (o cualquier otra 
persona) puede comprobar utilizando la clave p£blica del remitente para 
desencriptarla. De esta forma se prueba el verdadero origen del mensaje, 
y que no ha sido alterado por nadie, ya que s¢lo el remitente posee la 
clave secreta que ha producido esa firma. No es posible falsificar un 
mensaje firmado, y el remitente no podr  desautorizar su firma m s 
adelante.

Estos dos procesos pueden combinarse para obtener intimidad y 
autenticaci¢n al mismo tiempo, si se firma primero el mensaje con la 
clave secreta y se encripta despu‚s el mensaje firmado con la clave 
p£blica del destinatario. El destinatario sigue estos pasos en sentido 
contrario, al desencriptar primero el mensaje con su propia clave 
secreta y comprobar despu‚s la firma con la clave p£blica del remitente. 
El programa lo hace autom ticamente.

Como el algoritmo de cifrado en clave p£blica es mucho m s lento que el 
cifrado convencional de clave £nica, el proceso se realiza m s 
eficazmente con un algoritmo convencional r pido y de calidad, de clave 
£nica, para encriptar el mensaje. El mensaje original sin encriptar se 
denomina "texto normal"ø. Sin intervenci¢n del usuario, se utiliza una 
clave aleatoria temporal, generada s¢lo para esa "sesi¢n", para 
encriptar convencionalmente el fichero normal. Despu‚s se encripta esa 
clave aleatoria convencional con la clave p£blica del destinatario. La 
clave de la "sesi¢n" convencional, encriptada con esa clave p£blica, se 
env¡a al destinatario junto al texto cifrado. El destinatario recupera 
esa clave temporal con su propia clave secreta, y ejecuta con ella el 
algoritmo convencional de clave £nica, que es m s r pido, para 
desencriptar el mensaje cifrado.

Las claves p£blicas se guardan en "certificados de clave" individuales, 
que incluyen el identificador de usuario del propietario (el nombre de 
esa persona {y algun dato £nico, como la direcci¢n de correo-E}), un 
sello de hora del momento en el que se gener¢ el par y el material 
propio de la clave. Cada clave secreta est  encriptada con su propia 
contrase¤a, por si alguien la roba. Cada fichero de claves contiene uno 
o m s de esos certificados.

Las claves se identifican internamente mediante "identificador de 
clave", que es una "abreviatura" de la clave p£blica (sus 64 bits menos 
significativos). Cuando se muestra este identificador, s¢lo aparecen los 
24 bits inferiores, para mayor brevedad. Aunque muchas claves pueden 
compartir el mismo identificador de usuario, a efectos pr cticos no hay 
dos claves que compartan el mismo identificador de clave.

PGP utiliza "res£menes de mensaje" para elaborar las firmas. Un resumen 
de mensaje es una funci¢n "distribuci¢n" ("hash") unidireccional de 128 
bits, criptogr ficamente resistente, de ese mensaje. Es an logo en 
cierta manera a una "suma de verificaci¢n" o c¢digo CRC de comprobaci¢n 
de errores en que "representa" el mensaje de forma compacta y se utiliza 
para detectar cambios en ‚l. A diferencia de un CRC, sin embargo, le 
resultar  computacionalmente impracticable a un atacante idear un 
mensaje sustitutivo que produzca un resumen id‚ntico. El resumen del 
mensaje se encripta con la clave secreta para elaborar la firma.

Los documentos se firman a¤adi‚ndoles como prefijo un certificado de 
firma, con el identificador de la clave que se utiliz¢ para realizarla, 
un resumen de mensaje del documento (firmado con la clave secreta) y un 
sello de hora del momento de la firma. El destinatario utiliza el 
identificador de la clave para buscar la clave p£blica del remitente y 
comprobar la firma. El programa busca autom ticamente la clave p£blica y 
el identificador usuario en el fichero de claves correspondiente.

Los ficheros cifrados llevan como prefijo el identificador de la clave 
p£blica con la que se han encriptado. El destinatario utiliza este 
prefijo de identificaci¢n para encontrar la clave secreta y poder 
desencriptar el mensaje. Su programa busca autom ticamente la clave 
secreta en el fichero de claves correspondiente.

Estos dos tipos de fichero constituyen el m‚todo principal para 
almacenar y gestionar las claves p£blicas y secretas. En lugar de 
mantener las claves individuales en ficheros separados, se re£nen en 
ficheros para facilitar la b£squeda autom tica, ya sea por identificador 
de clave o por identificador de usuario. Cada usuario mantiene su propio 
par de ficheros. Las claves p£blicas individuales se mantienen en 
ficheros aparte durante el tiempo necesario para enviarlas a alg£n 
amigo, que las a¤adir  entonces a su propio fichero de claves.


Instalaci¢n de PGP
==================

El lanzamiento para MSDOS de PGP 2.3 va en un archivo comprimido llamado 
PGP23.ZIP (cada nuevo lanzamiento tendr  un nombre como "PGPxy.ZIP", 
indicando la versi¢n de PGP n£mero x.y). El archivo puede descomprimirse 
con la utilidad compartida de descompresi¢n PKUNZIP, o con la utilidad 
"unzip" de Unix. El lanzamiento de PGP contiene un fichero llamado 
LEAME.DOC que deber¡a leer antes de instalar PGP. Este fichero LEAME.DOC 
contiene las £ltimas noticias acerca de las novedades en el lanzamiento 
de PGP, adem s de informaci¢n sobre qu‚ hay en los otros ficheros 
incluidos.

Si tiene todav¡a la versi¢n 1.0 de PGP para MSDOS, casi deber¡a 
borrarla, porque ya nadie la utiliza. Si no quiere hacerlo, renombre el 
antiguo fichero ejecutable como pgp1.exe, para evitar conflictos con el 
nuevo PGP.

Para instalar PGP en su sistema MSDOS, s¢lo tiene que copiar el archivo 
comprimido PGPxx.ZIP en un directorio apropiado del disco duro (como 
C:\PGP), y descomprimirlo con PKUNZIP. Para obtener un resultado ¢ptimo, 
tiene tambi‚n que modificar el fichero AUTOEXEC.BAT, tal como se 
describe en otra parte del manual; pero eso puede esperar a que haya 
jugado un poco con PGP y haya le¡do algo m s. Si no ha ejecutado PGP 
antes, el primer paso despu‚s de instalarlo (y leer este manual) es 
ejecutar la orden de generaci¢n de claves de PGP "pgp -kg".

La instalaci¢n en Unix y VAX/VMS es parecida a la instalaci¢n en MSDOS, 
pero tendr  que compilar antes el c¢digo fuente. Para ello se incluye un 
"makefile" de Unix con ese c¢digo.

Para obtener m s informaci¢n sobre este tema, consulte la Gu¡a de 
instalaci¢n de PGP en el fichero INSTALAR.DOC incluido con esta versi¢n. 
Describe en detalle c¢mo configurar el directorio PGP y el fichero 
AUTOEXEC.BAT, y c¢mo utilizar PKUNZIP para instalarlo.


C¢mo utilizar PGP
=================

Para ver un resumen de las instrucciones
=---------------------------------------

Para ver un resumen r pido de las instrucciones de PGP, escriba:

     pgp -h


Encriptaci¢n de un mensaje
=-------------------------

Para encriptar un fichero de texto normal con la clave p£blica del 
destinatario, escriba:

     pgp -e ftexto identificador_des

Esta orden produce un texto cifrado llamado ftexto.pgp. Un ejemplo 
podr¡a ser:

     pgp -e carta.txt Alice
o:
     pgp -e carta.txt "Alice S"

El primer ejemplo busca en el fichero de claves p£blicas "pubring.pgp" 
alg£n certificado que contenga la cadena de caracteres "Alice" en el 
campo del identificador de usuario. El segundo ejemplo encontrar  
cualquier identificador que contenga "Alice S". No se pueden incluir 
espacios en la cadena dentro de la l¡nea de ¢rdenes, a menos que sea 
entre comillas. La b£squeda no tiene en cuenta el tipo de letra 
(may£sculas o min£sculas). Si se encuentra una clave p£blica que 
coincide, PGP la utiliza para encriptar el fichero normal "carta.txt", 
produciendo un fichero cifrado llamado "carta.pgp".

PGP intenta comprimir el texto normal antes de encriptarlo, mejorando 
considerablemente su resistencia al criptoan lisis. Por esta raz¢n, el 
fichero cifrado ser  probablemente menor que el fichero normal.

Si quiere enviar el mensaje cifrado por canales de correo-E, convi‚rtalo 
al formato ASCII imprimible "radix-64" a¤adiendo la opci¢n -a, tal como 
se describe m s adelante.


Encriptaci¢n de un mensaje para m£ltiples destinatarios
=------------------------------------------------------

Si quiere enviar el mismo mensaje a m s de una persona, puede hacer que 
se encripte para varios destinatarios, cualquiera de los cuales podr¡a 
desencriptar el mismo fichero. Para indicar m£ltiples destinatarios, 
s¢lo tiene que a¤adir m s identificadores en la l¡nea de ¢rdenes, como:

     pgp -e carta.txt Alice Bob Carol

As¡ se crear¡a un fichero cifrado llamado carta.pgp que podr¡a 
desencriptar Alice, Bob o Carol. Puede indicarse cualquier n£mero de 
destinatarios.


Firma de un mensaje
=------------------

Para firmar un fichero normal con su clave secreta, escriba:

     pgp -s ftexto [-u su_identificador]

N¢tese que los [corchetes] indican un campo opcional, por lo que no 
deben escribirse.

Esta orden produce un fichero firmado llamado ftexto.pgp. Un ejemplo 
podr¡a ser:

     pgp -s carta.txt -u Bob

Esta orden buscar¡a en su fichero de claves secretas "secring.pgp" 
cualquier certificado que contenga la cadena "Bob" en el campo del 
identificador de usuario. La b£squeda no tiene en cuenta el tipo de 
letra. Si se encuentra una clave secreta que coincida, PGP la utiliza 
para firmar el fichero normal "carta.txt", produciendo un fichero 
firmado "carta.pgp".

Si no se incluye el campo de identificador de usuario, se utiliza para 
la firma la clave m s reciente del fichero de claves secretas como 
elecci¢n por omisi¢n.


Firma y posterior encriptaci¢n
=-----------------------------

Para firmar un fichero normal con su clave secreta, y despu‚s 
encriptarlo con la clave p£blica del destinatario:

     pgp -es ftexto identificador_des [-u su_identificador]

N¢tese que los [corchetes] indican un campo opcional, por lo que no 
deben escribirse.

Este ejemplo produce un fichero cifrado anidado, ftexto.pgp. La clave 
secreta para firmar se busca autom ticamente en el fichero 
correspondiente por medio de su_identificador. La clave p£blica de 
encriptaci¢n para el destinatario se busca autom ticamente en el fichero 
de claves p£blicas por medio de identificador_des. Si no incluye el 
identificador de usuario del destinatario, se le pedir  que lo haga.

Si no indica su propio campo de identificador de usuario, se utiliza la 
clave m s reciente del fichero de claves secretas como elecci¢n por 
omisi¢n para la firma.

N¢tese que PGP intentar  comprimir el texto normal antes de encriptarlo.

Si quiere enviar este mensaje cifrado por medio de canales de correo-E, 
convi‚rtalo al formato ASCII imprimible "radix-64" a¤adiendo la opci¢n 
- -a, tal como se describe m s adelante.

Pueden indicarse m£ltiples destinatarios a¤adiendo m s identificadores 
en la l¡nea de ¢rdenes.


Utilizaci¢n de encriptaci¢n convencional exclusivamente
=------------------------------------------------------

Algunas veces s¢lo hace falta encriptar un fichero al estilo antiguo, 
con criptograf¡a convencional de clave £nica. Este sistema resulta 
conveniente para proteger aquellos ficheros de archivo que se vayan a 
almacenar, sin enviar a nadie. Dado que va a desencriptar el fichero la 
misma persona que lo ha encriptado, no se necesita realmente 
criptograf¡a de clave p£blica.

Para encriptar un fichero normal s¢lo con criptograf¡a convencional, 
escriba:

     pgp -c ftexto

Este ejemplo encripta el fichero normal ftexto, produciendo un fichero 
cifrado ftexto.pgp, sin criptograf¡a de clave p£blica, ficheros de 
claves, identificadores de usuario ni nada de eso. Pedir  una frase de 
contrase¤aø como clave convencional para encriptar el fichero. Esta 
frase no tiene por qu‚ ser (de hecho, NO DEBERIA ser) la misma que 
utiliza para proteger su propia clave secreta. N¢tese que PGP intentar  
comprimir el fichero antes de encriptarlo.

PGP nunca encripta el mismo texto normal dos veces de la misma forma, 
incluso con la misma frase de contrase¤a.


Desencriptaci¢n y comprobaci¢n de firmas
=---------------------------------------

Para desencriptar un fichero cifrado, o para comprobar la integridad de 
un fichero firmado:

     pgp fcifrado [-o fnormal]

N¢tese que los [corchetes] indican un campo opcional, por lo que no 
deben escribirse.

Se asume que, por omisi¢n, el nombre del fichero cifrado tiene una 
extensi¢n ".pgp". El nombre opcional de salida para el texto normal 
indica d¢nde hay que poner el texto procesado. Si no se indica ning£n 
nombre, se utiliza el mismo del fichero cifrado, sin extensi¢n. Si se 
anida una firma dentro de un fichero cifrado, esta se desencripta 
autom ticamente y se comprueba su integridad. Se mostrar  el 
identificador completo del firmante.

N¢tese que el "desembalaje" del fichero cifrado es completamente 
autom tico, sin importar si est  firmado, cifrado o ambas cosas. PGP 
utiliza el prefijo del identificador de clave en el fichero cifrado para 
encontrar autom ticamente la clave secreta en el fichero. Si hay una 
firma anidada, PGP usa el prefijo de identificador de esa clave para 
encontrar autom ticamente la clave p£blica en el fichero correspondiente 
y comprobarla. Si las claves adecuadas est n en los ficheros de claves 
no se requiere m s intervenci¢n, excepto para dar la contrase¤a de la 
clave secreta, en su caso. Si el fichero hab¡a sido encriptado sin 
criptograf¡a de clave p£blica, PGP lo reconoce y pide la contrase¤a para 
desencriptarlo convencionalmente.



Gesti¢n de claves
=================

Desde los tiempos de Julio C‚sar, la gesti¢n de claves ha sido el 
aspecto m s dif¡cil de la criptograf¡a. Una de las caracter¡sticas que 
distinguen a PGP es su completa gesti¢n de claves.


Generaci¢n de claves RSA
=-----------------------

Para generar su propio par £nico de claves p£blica/secreta de un tama¤o 
determinado, escriba:

     pgp -kg

PGP le mostrar  un men£ de tama¤os recomendados para la clave (nivel 
aficionado, nivel comercial y nivel militar), y le pedir  qu‚ indique 
qu‚ tama¤o de clave quiere, hasta aproximadamente mil bits. Cuanto m s 
grande es la clave, mayor es la seguridad que se obtiene, pero el precio 
es una disminuci¢n de la velocidad.

Tambi‚n se pide un identificador de usuario, esto es, su nombre. Resulta 
conveniente poner como identificador un nombre completo, porque as¡ hay 
menor probabilidad de que otras personas elijan una clave p£blica 
equivocada para encriptar los mensajes dirigidos a usted. En el 
identificador de usuario se permiten espacios y signos de puntuaci¢n. 
Tambi‚n conviene poner su direcci¢n de correo-E entre < ngulos> despu‚s 
del nombre, como en:

     Juan Espannol <juan@basa.org>

Si no tiene direcci¢n de correo-E, ponga su n£mero de tel‚fono u otra 
informaci¢n que ayude a garantizar la unicidad de su identificador.

PGP tambi‚n pedir  una "frase de contrase¤a" para proteger su clave 
secreta en caso de que caiga en otras manos. Nadie podr  utilizar su 
clave secreta sin esta frase. La frase de contrase¤a es como una 
contrase¤a habitual, excepto en que puede ser una expresi¢n o frase con 
varias palabras, espacios, signos de puntuaci¢n o cualquier otra cosa 
que quiera poner. No la pierda, porque no hay forma de recuperarla. La 
frase de contrase¤a le har  falta m s adelante, cada vez que utilice su 
clave secreta. Se tiene en cuenta el tipo de letra (may£scula o 
min£scula), y la frase no debe ser ni demasiado corta ni f cil de 
adivinar. Nunca aparece en la pantalla. No la deje escrita donde alguien 
pueda verla, ni la almacene en su ordenador. Si no quiere tener frase de 
contrase¤a (­no sea tonto!), simplemente pulse retorno (enter) en el 
indicador correspondiente.

El par de claves p£blica/secreta se deriva de grandes n£meros 
verdaderamente aleatorios, obtenidos al medir intervalos de tiempo entre 
pulsaciones de tecla, con un temporizador r pido. El programa le pedir  
que introduzca un texto al azar para poder acumular algunos bits 
aleatorios para las claves. Cuando se le pida, debe pulsar algunas 
teclas razonablemente al azar, y no vendr¡a mal que tambi‚n su contenido 
fuera irregular. Parte de la aleatoriedad se deriva de la 
impredictibilidad del contenido de lo que escribe. Por lo tanto, no 
escriba secuencias repetidas de caracteres.

Tenga en cuenta que la generaci¢n de claves RSA es un proceso MUY lento. 
Puede llevar unos segundos para una clave peque¤a en un procesador 
r pido, o varios minutos para una clave larga en un viejo IBM PC/XT.

El par de claves generado se colocar  en sus ficheros de claves p£blicas 
y secretas. Puede utilizar m s adelante la orden -kx y extraer (copiar) 
su nueva clave p£blica desde el fichero correspondiente, para ponerla en 
un fichero de clave separado, listo para distribuir entre sus amigos. 
Podr  enviar este fichero para que lo incluyan en sus ficheros de claves 
p£blicas. Naturalmente, la clave secreta es para usted, y deber¡a 
incluirlo en su fichero de claves secretas. Cada clave secreta est  
protegida individualmente por su propia contrase¤a.

Nunca d‚ su clave secreta a nadie. Por la misma raz¢n, no haga pares de 
claves para sus amigos. Cada uno debe hacer el suyo. Mantenga siempre 
control f¡sico sobre su clave secreta, y no se arriesgue a exponerla 
almacen ndola en un ordenador remoto compartido. Cons‚rvela en su propio 
ordenador personal.


Adici¢n de una clave al fichero
=------------------------------

Para a¤adir el contenido de un fichero de claves p£blicas o secretas al 
fichero de claves correspondiente (n¢tese que los [corchetes] indican un 
campo opcional):

     pgp -ka fdclaves [fichero]

La extensi¢n por omisi¢n del fichero es ".pgp". El nombre opcional del 
fichero es, por omisi¢n, "pubring.pgp" o "secring.pgp", seg£n se refiera 
a claves p£blicas o secretas. Puede indicarse un nombre diferente para 
el fichero, y tambi‚n su extensi¢n por omisi¢n ser  ".pgp".

Si la clave ya est  en el fichero, PGP no la a¤ade otra vez. Se incluyen 
todas las claves del fichero, excepto las duplicadas. Si la clave para 
a¤adir incluye firmas que la certifican, se incorporan junto con ella. 
Si ya se encontraba en el fichero, PGP solamente a¤adir  las firmas que 
no estuviesen.


Supresi¢n de una clave del fichero
=---------------------------------

Para suprimir una clave del fichero de claves p£blicas:

     pgp -kr identificador [fichero]

Este proceso busca en el fichero el identificador indicado, y lo suprime 
si encuentra una coincidencia. Recuerde que cualquier fragmento del 
identificador basta para que haya esa coincidencia. Se asume que 
"pubring.pgp" es literalmente ese nombre opcional del fichero. Puede 
omitirlo, o puede indicar "secring.pgp" si quiere suprimir una clave 
secreta. Puede dar tambi‚n un nombre distinto para el fichero de claves. 
La extensi¢n por omisi¢n es ".pgp".

Si hay m s de un identificador de usuario para esa clave, se le 
preguntar  si s¢lo quiere eliminar el identificador indicado, dejando la 
clave y los otros identificadores intactos.


Extracci¢n (copia) de una clave del fichero
=-----------------------------------------

Para extraer (copiar) una clave del fichero de claves p£blicas o 
secretas:

     pgp -kx identificador fdclaves [fichero]

Este proceso copia (sin borrar) la clave especificada por el 
identificador desde el fichero al fichero indicado. Resulta 
especialmente £til para dar a alguien una copia de su clave p£blica.

Si la clave tiene alguna firma de certificaci¢n, tambi‚n se copia.

Si quiere que la clave extra¡da se represente en caracteres ASCII 
imprimibles, para correo-E, ponga las opciones -kxa.


Visualizaci¢n del contenido del fichero
=--------------------------------------

Para ver el contenido del fichero de claves p£blicas:

     pgp -kv[v] [identificador] [fichero]

Muestra la lista con aquellas claves del fichero que coinciden con la 
subcadena especificada como identificador. Si omite el identificador, se 
muestran todas las claves. Se asume que "pubring.pgp" es ese nombre 
opcional de fichero. Puede omitirlo, o indicar "secring.pgp" si quiere 
ver la lista de claves secretas. Tambi‚n puede, si lo desea, especificar 
otro nombre distinto para el fichero de claves. La extensi¢n por omisi¢n 
es ".pgp".

Para ver las firmas de certificaci¢n de cada clave, utilice la opci¢n 
- -kvv:

     pgp -kvv [identificador] [fichero]

Si quiere especificar un nombre concreto de fichero, viendo todas las 
claves en ‚l, pruebe esta forma alternativa:

     pgp fdclaves

Sin indicar ninguna opci¢n, PGP muestra la lista de todas las claves en 
fdclaves.pgp, intentando adem s a¤adirlas al fichero de claves si no 
estuviesen.


C¢mo proteger las claves p£blicas contra manipulaci¢n
=----------------------------------------------------

En un sistema de clave p£blica no hay que proteger las claves p£blicas 
contra exposici¢n. De hecho, es mejor que est‚n ampliamente difundidas. 
Sin embargo, es importante protegerlas contra manipulaci¢n, para 
asegurar que una clave pertenece realmente a quien parece pertenecer. 
Este quiz  sea el punto m s vulnerable de un criptosistema de clave 
p£blica. Veamos primero un posible desastre, y despu‚s c¢mo evitarlo con 
PGP.

Supongamos que usted quiere enviar a Alice un mensaje privado. Recibeø 
la clave p£blica de Alice desde una BBS (Bulletin Board System: tabl¢n 
electr¢nico de anuncios). Encripta la carta para Alice con esa clave, y 
se la env¡a por medio del correo-E de la BBS.

Desafortunadamente, sin saberlo Alice ni usted, otro usuario llamado 
Charlie se ha infiltrado en la BBS, y ha generado una clave p£blica 
propia que lleva el identificador de usuario de Alicia. Pone 
secretamente esa clave falsa en lugar de la verdadera. Usted, sin 
saberlo, utiliza esa clave en lugar de la aut‚ntica. Todo parece normal, 
porque la clave falsa tiene el identificador de usuario de Alice. 
Entonces, Charlie puede descifrar el mensaje dirigido a Alice, porque 
tiene la clave secreta correspondiente. Puede incluso volver a encriptar 
el mensaje con la verdadera clave p£blica de Alice, y envi rselo a ella 
para que nadie sospeche nada. A£n m s, puede incluso hacer firmas en 
nombre de Alice con esa clave secreta, porque todo el mundo utiliza la 
clave p£blica falsa para comprobar las firmas de Alice.

La £nica forma de evitar este desastre es impedir que alguien pueda 
manipular las claves p£blicas. Si la clave p£blica de Alice se ha 
obtenido directamente de ella, no hay problema. Sin embargo, esto puede 
resultar dif¡cil si la persona se encuentra a mil kil¢metros, o no es 
localizable en ese momento.

Podr¡a conseguir la clave p£blica de Alice de un amigo en el que conf¡en 
ambos, David, que sabe que su copia de la clave p£blica de Alice es 
buena. David podr¡a firmar la clave p£blica de Alice, respondiendo de la 
integridad de la clave. David realizar¡a esta firma con su propia clave 
secreta.

De esta forma se crear¡a un certificado firmado de clave p£blica, que 
demostrar¡a que la clave de Alice no ha sido manipulada. Este mecanismo 
requiere que usted tenga una copia buena de la clave p£blica de David, 
para poder comprobar la firma. David podr¡a tambi‚n proporcionar a Alice 
una copia firmada de la clave p£blica de usted. David hace, por tanto, 
de referencia entre usted y Alice.

David o Alice podr¡an enviar a la BBS ese certificado firmado de clave 
p£blica de parte de Alice, y usted podr¡a recibirlo m s adelante. 
Entonces podr¡a comprobar la firma con la clave p£blica de David, y 
asegurarse de que es la verdadera clave de Alice. Ning£n impostor podr¡a 
hacer que aceptase una clave falsa como si fuera de Alice, porque nadie 
puede falsificar la firma de David.

Una persona de amplia confianza podr¡a incluso especializarse en ofrecer 
este servicio de "referencia" entre usuarios, proporcionando firmas para 
esos certificados de clave p£blica. Esta persona de confianza podr¡a 
considerarse un "organizador de claves", o "autoridad de certificaci¢n". 
Podr¡a confiarse en que cualquier certificado de clave p£blica con la 
firma del organizador pertenecer¡a verdaderamente a quien parec¡a 
pertenecer. Los usuarios que quisieran participar s¢lo necesitar¡an una 
copia buena de la clave p£blica del organizador para poder verificar sus 
firmas.

Un organizador centralizado de claves, o autoridad de certificaci¢n, 
est  especialmente indicado en grandes instituciones gubernativas o 
empresariales con control centralizado. Algunos entornos institucionales 
ya utilizan jerarqu¡as de autoridades de certificaci¢n.

Para entornos de base m s descentralizados, estilo "guerrilla", 
probablemente funcionar  mejor que un organizador centralizado el 
permitir a cualquier usuario actuar como referencia de confianza de sus 
amigos. PGP tiende a enfatizar este enfoque org nico descentralizado, no 
institucional. Refleja mejor la forma natural que tienen los humanos de 
interaccionar personalmente a nivel social, y permite a la gente elegir 
mejor en qui‚n confiar para la gesti¢n de claves.

Todo este tema de proteger las claves p£blicas contra manipulaci¢n es el 
problema individual m s dif¡cil con que se encuentra la aplicaci¢n 
pr ctica de la clave p£blica. Es el "tal¢n de Aquiles" de la 
criptograf¡a de clave p£blica, y solamente en resolver este problema hay 
invertida una gran complejidad de programaci¢n.

S¢lo deber¡a utilizar una clave p£blica despu‚s de comprobar que es una 
clave aut‚ntica no manipulada, y que pertenece a la persona a la que 
dice pertenecer. Puede estar seguro de ello si obtiene el certificado de 
clave p£blica directamente de su propietario, o si lleva la firma de 
alguien en quien conf¡a, y del que ya se tiene una clave p£blica 
aut‚ntica. Por otra parte, el identificador de usuario deber¡a llevar el 
nombre completo del propietario, no s¢lo su nombre de pila.

Por mucho que tenga la tentaci¢n-- y la tendr --, nunca, NUNCA, ceda a 
la comodidad y se f¡e de una clave p£blica que haya recibido de una BBS, 
a menos que vaya firmada por alguien en quien usted conf¡a. Esa clave 
p£blica sin certificar puede haber sido manipulada por cualquiera, quiz  
incluso el mismo administrador de la BBS.

Si le piden que firme el certificado de la clave p£blica de alguien, 
compruebe que realmente pertenece a la persona mencionada en el 
identificador de usuario. Su firma en ese certificado de clave p£blica 
es una promesa que usted hace de que la clave pertenece realmente a esa 
persona. La gente que conf¡e en usted aceptar  esa clave p£blica porque 
lleva su firma. No es recomendable fiarse de rumores-- no firme la clave 
a menos que tenga conocimiento independiente y de primera mano de que 
realmente pertenece a esa persona. Preferiblemente, solamente deber¡a 
firmarla si la ha recibido directamente de ella.

Debe estar mucho m s seguro de qui‚n es el propietario de una clave 
p£blica para firmarla que para encriptar un mensaje. Para estar lo 
bastante convencido de la validez de una firma como para utilizarla, 
deber¡an valer las firmas de certificaci¢n de las referencias de 
confianza. Sin embargo, para firmar una clave usted mismo, debe tener 
conocimiento independiente y de primera mano de qui‚n es el propietario 
de esa clave. Podr¡a llamar al propietario por tel‚fono y leerle el 
fichero de clave, para que confirme que la clave es verdaderamente 
suya-- compruebe que est  hablando con la persona indicada. Consulte la 
secci¢n llamada "Verificaci¢n de una clave p£blica por tel‚fono" en el 
volumen de Temas especiales para obtener m s informaci¢n.

Tenga en cuenta que la firma en un certificado de clave p£blica no 
responde de la integridad de esa persona, solamente de la integridad (la 
pertenencia) de la clave p£blica de esa persona. Usted no arriesga su 
propia credibilidad firmando la clave p£blica de un soci¢pata, siempre 
que est‚ completamente seguro de que la clave le pertenece a ‚l. Otra 
gente aceptar  que le pertenece porque usted la ha firmado (asumiendo 
que conf¡en en usted), pero no se fiar n del propietario de esa clave. 
Confiar en una clave no es lo mismo que confiar en su propietario.

La confianza no es necesariamente transferible; tengo un amigo del que 
s‚ que no miente. Es un cr‚dulo que cree que el presidenteø no miente. 
Eso no quiere decir que yo crea que el presidente no miente. Es s¢lo 
sentido com£n. Si me f¡o de la firma de Alicia en una clave, y Alicia se 
f¡a de la firma de Charlie, eso no implica que yo me tenga que fiar de 
la firma de Charlie.

Resulta conveniente mantener su propia clave p£blica a mano, con una 
colecci¢n de firmas de certificaci¢n de diversas "referencias", para que 
la mayor¡a de la gente conf¡e al menos en una de las que responden de la 
validez de su clave. Puede enviar la clave con su colecci¢n de firmas de 
certificaci¢n a varias BBSs. Si firma la clave p£blica de alguien, 
devu‚lvasela con la firma, para que pueda a¤adirla a la colecci¢n de 
credenciales de su clave.

PGP controla qu‚ claves del fichero de claves p£blicas han sido 
certificadas adecuadamente con firmas de referencias en las que usted 
conf¡a. Todo lo que tiene que hacer es decir a PGP en qu‚ personas 
conf¡a como referencias, y certificar esas claves con la suya propia, 
que es fundamentalmente fiable. PGP puede continuar desde ah¡, validando 
cualquier clave firmada por esas referencias designadas. Adem s, por 
supuesto, puede firmar m s claves usted mismo. Seguiremos con esto m s 
adelante.

Aseg£rese de que nadie pueda manipular su fichero de claves p£blicas. La 
comprobaci¢n de cualquier nuevo certificado firmado de clave p£blica 
depende en £ltima instancia de la integridad de las claves de confianza 
que ya hay en el fichero de claves. Mantenga control f¡sico sobre el 
fichero de claves p£blicas, preferiblemente en su propio ordenador 
personal, en lugar de hacerlo en un sistema remoto multiusuario, como lo 
har¡a con su clave secreta. El objetivo es protegerlo contra 
manipulaci¢n, no contra exposici¢n. Conserve una copia de seguridad 
fiable de los ficheros de claves p£blicas y secretas en un material 
protegido contra escritura. 

Como su propia clave es la m xima autoridad para certificar directa o 
indirectamente las claves de su fichero, es la que m s tiene que 
proteger contra manipulaci¢n. Para detectar cualquier manipulaci¢n de su 
propia clave p£blica, fundamentalmente fiable, PGP puede configurarse 
para que la compare autom ticamente con una copia de seguridad en 
material protegido contra escritura. Para obtener m s informaci¢n, 
consulte la descripci¢n de la orden "-kc" de comprobaci¢n de ficheros en 
el volumen sobre Temas especiales.

PGP generalmente asume que usted va a mantener seguridad f¡sica sobre el 
sistema y los ficheros de claves, y sobre la copia misma de PGP. Si un 
intruso pudiese manipular su disco, entonces podr¡a en teor¡a manipular 
el mismo PGP, dejando en entredicho cualquier sistema de seguridad que 
pueda tener PGP para detectar la manipulaci¢n de claves.

Una forma algo complicada de proteger el fichero completo de claves 
p£blicas contra manipulaci¢n es firmarlo con su propia clave secreta. 
Puede hacerlo elaborando un certificado separado de firma para el 
fichero, con las opciones "-sb" (v‚ase la secci¢n llamada "Separaci¢n de 
firmas de los mensajes" en la Gu¡a del usuario de PGP, volumen de Temas 
especiales). Desafortunadamente, sigue siendo necesario mantener una 
copia aparte de su propia clave p£blica, para comprobar la firma que ha 
hecho. No puede fiarse para comprobar la firma de la clave almacenada en 
el fichero de claves p£blicas, ya que es precisamente parte de lo que 
intenta comprobar.


¨C¢mo controla PGP la validez de las claves?
=-------------------------------------------

Antes de empezar esta secci¢n, lea la anterior sobre "C¢mo proteger las 
claves p£blicas contra manipulaci¢n".

PGP controla qu‚ claves en el fichero de claves p£blicas han sido 
certificadas adecuadamente con firmas de referencias de confianza. Todo 
lo que tiene que hacer usted es decir a PGP en qu‚ personas conf¡a como 
referencia, y certificar esas claves con la suya propia, que es 
fundamentalmente fiable. PGP puede continuar desde ah¡, validando 
cualquier otra clave que haya sido firmada por esas referencias 
elegidas. Por supuesto, usted mismo puede firmar m s claves.

Hay dos criterios completamente distintos por los que PGP juzga la 
utilidad de una clave p£blica-- no los confunda:

  1) ¨Pertenece la clave realmente a quien parece pertenecer?
     En otras palabras, ¨ha sido certificada con una firma de confianza?
  2) ¨Pertenece a alguien en quien podemos confiar para certificar otras
     claves?

PGP puede calcular la respuesta a la primera pregunta. Para responder a 
la segunda, usted, el usuario, debe informar a PGP expl¡citamente. 
Cuando se da la respuesta a la pregunta 2, PGP puede calcular la 
respuesta a la pregunta 1 para otras claves que hayan sido firmadas por 
esa referencia designada como fiable.

Las claves que han sido certificadas por una referencia de confianza ya 
se consideran v lidas en PGP. Las claves de esas referencias deben estar 
certificadas por usted u otra referencia de confianza.

PGP tambi‚n permite tener distintos m rgenes de confianza para las 
personas que van a actuar como referencia. La confianza en el 
propietario de una clave para servir de referencia no refleja 
simplemente la estimaci¢n de su integridad personal-- tambi‚n deber¡a 
reflejar cu l cree usted que es su nivel de comprensi¢n de la gesti¢n de 
claves, y de su buen juicio en la firma de estas. Puede usted designar 
una persona en PGP como desconocida, no fiable, de relativa confianza, o 
de completa confianza para certificar otras claves p£blicas. Esta 
informaci¢n se almacena en el fichero junto con la clave de esa persona, 
pero, al indicar a PGP que copie una clave, no se incluye con ella, ya 
que esas opiniones privadas sobre confianza se consideran 
confidenciales.

Cuando PGP est  calculando la validez de una clave p£blica, examina el 
nivel de confianza de todas las firmas incluidas. Elabora una puntuaci¢n 
proporcional de validez-- dos firmas relativamente fiables se consideran 
tan cre¡bles como una completamente fiable. El escepticismo de PGP es 
ajustable-- por ejemplo, puede establecerse que hagan falta dos firmas 
completamente fiables, o tres relativamente fiables, para dar una clave 
por v lida.

Su propia clave es "axiom ticamente" v lida para PGP, y no necesita 
ninguna firma de referencia para probar su validez. PGP sabe qu‚ claves 
p£blicas son de usted buscando las claves secretas correspondientes en 
el otro fichero. PGP tambi‚n asume que usted conf¡a completamente en s¡ 
mismo para certificar otras claves.

Seg£n pase el tiempo, ir  acumulando claves de otras personas, a las que 
puede designar como referencias de confianza. Cada uno ir  eligiendo sus 
propias referencias. Y cada uno ir  gradualmente acumulando y 
distribuyendo con su clave una colecci¢n de firmas de certificaci¢n, 
esperando as¡ que cualquiera que la reciba conf¡e al menos en una o dos 
de ellas. Se producir  as¡ la aparici¢n de una red descentralizada de 
confianza para las claves p£blicas, resistente a fallos.

Este enfoque de base, £nico, contrasta claramente con los esquemas 
habituales del Gobierno para gestionar claves p£blicas, como el Internet 
Privacy Enhanced Mail (PEM) {Correo mejorado en intimidad para 
Internet}, que se fundamentan en un control centralizado y una confianza 
centralizada y obligatoria. Los esquemas habituales conf¡an en una 
jerarqu¡a de Autoridades de certificaci¢n que dictan en qui‚n debe usted 
confiar. El m‚todo probabil¡stico y descentralizado de PGP para 
determinar la legitimidad de las claves p£blicas es la piedra angular de 
su arquitectura de gesti¢n de claves. PGP le permite que elija usted 
mismo en qui‚n confiar, y le pone en el v‚rtice de su propia pir mide 
personal de certificaci¢n. PGP es para personas que prefieren preparar 
sus propios paraca¡das.


C¢mo proteger las claves secretas contra revelaci¢n
=--------------------------------------------------

Proteja con cuidado su propia clave y su frase de contrase¤a. Con mucho, 
mucho cuidado. Si su clave secreta se ve alguna vez comprometidaø, es 
mejor que corra la voz r pidamente y se lo diga a todas las partes 
interesadas (buena suerte...) antes de que alguien la utilice para hacer 
firmas en su nombre. Por ejemplo, podr¡an firmar certificados falsos de 
clave p£blica, lo que podr¡a causar problemas a muchas personas, 
especialmente si su firma tiene amplio reconocimiento. Por supuesto, el 
compromiso de su propia clave secreta podr¡a poner al descubierto todos 
los mensajes dirigidos a usted.

Para proteger su clave secreta, puede empezar por mantener siempre 
control f¡sico sobre ella. Es suficiente con tenerla en el ordenador 
personal en casa, o en un port til que pueda llevar con usted. Si tiene 
que utilizar un ordenador de la oficina, sobre el que no siempre tiene 
control f¡sico, lleve sus ficheros de claves p£blicas y secretas en un 
disco extra¡ble, y nunca se lo deje. No es conveniente permitir que la 
clave secreta se encuentre en un ordenador remoto multiusuario, como por 
ejemplo un sistema Unix con acceso telef¢nico. Alguien podr¡a fisgonear 
en la l¡nea del modem y conseguir la frase de contrase¤a, y despu‚s 
conseguir la clave secreta del sistema. S¢lo deber¡a utilizar la clave 
secreta en una m quina sobre la que tenga control f¡sico.

No guarde su contrase¤a dentro del mismo ordenador que tiene el fichero 
con las claves secretas. Guardar la clave secreta y la contrase¤a en el 
mismo ordenador es tan peligroso como guardar su n£mero secreto en la 
misma cartera que la tarjeta del cajero autom tico. Usted no querr  que 
alguien ponga sus manos en el disco que contiene la frase de contrase¤a 
y el fichero de claves secretas. Ser¡a m s seguro que memorizase la 
frase, y que no la guardase en ning£n sitio excepto en su cerebro. Si 
cree que debe escribirla, mant‚ngala protegida, quiz  incluso mejor que 
el fichero de las claves secretas.

Mantenga copias de seguridad del fichero de claves secretas-- recuerde, 
usted tiene la £nica copia de su clave secreta, y perderla inutilizar¡a 
todas las copias de su clave p£blica que haya por el mundo.

El enfoque descentralizado y no institucional que utiliza PGP para 
gestionar las claves p£blicas tiene sus ventajas, pero, 
desafortunadamente, tambi‚n implica que no se puede confiar en una lista 
£nica de las claves comprometidas. Por lo tanto, resulta m s dif¡cil 
controlar el da¤o que puede causar el compromiso de una clave. S¢lo 
puede divulgar la noticia y confiar en que todo el mundo se entere.

Si ocurre lo peor-- tanto su clave secreta como la frase de contrase¤a 
se ven comprometidos (esperemos que llegue a enterarse por alg£n medio) 
- --, tendr  que emitir un certificado de "compromiso de clave". Este tipo 
de certificado se utiliza para advertir a los dem s de que dejen de 
utilizar su clave p£blica. Puede hacer que PGP elabore ese certificado 
mediante la orden "-kd". Despu‚s tiene que enviarlo al resto de los 
habitantes del planeta, o al menos a todos sus amigos y los amigos de 
sus amigos, etc‚tera. Sus propios programas PGP instalar n ese 
certificado de compromiso en sus ficheros de claves p£blicas, y evitar  
que utilicen la clave por error. Puede entonces generar un nuevo par de 
claves secreta/p£blica, y distribuir la nueva clave p£blica. Puede 
enviar en un solo lote la nueva clave con el certificado de compromiso 
de la antigua.


Revocaci¢n de una clave p£blica
=------------------------------

Suponga que, por alg£n motivo, tanto su clave secreta como su contrase¤a 
se ven comprometidas. Tendr  que dec¡rselo al resto del mundo, para que 
dejen de utilizar su clave p£blica. Para ello, tiene que emitir un 
certificado de "compromiso de clave" y revocar la clave p£blica.

Para generar ese certificado, utilice la orden -kd:

     pgp -kd su_identificador

Este certificado lleva su firma, hecha con la misma clave que est  
revocando. Deber¡a distribuirlo ampliamente cuanto antes. Las personas 
que lo reciban podr n a¤adirlo a sus ficheros de claves p£blicas, y sus 
programas PGP evitar n autom ticamente que vuelvan a utilizar la clave 
antigua por error. Puede generar un nuevo par de claves secreta/p£blica, 
y publicar la nueva clave p£blica.

Puede tambi‚n querer revocar la clave por alg£n otro motivo. Se emplea 
el mismo mecanismo.


¨Qu‚ pasa si pierde la clave secreta?
- -------------------------------------

Normalmente, si quiere revocar la clave secreta puede utilizar la orden 
"-kd" para emitir un certificado de revocaci¢n, firmado con su propia 
clave secreta (v‚ase "Revocaci¢n de una clave p£blica").

Pero ¨qu‚ puede hacer si pierde la clave secreta, o si se destruye?  No 
puede revocarla usted mismo, porque debe utilizar la clave secreta para 
hacerlo, y ya no la tiene. Una versi¢n posterior de PGP ofrecer  una 
forma m s segura de revocar clave bajo esas circunstancias, permitiendo 
que referencias de confianza certifiquen que una clave p£blica ha sido 
revocada. Por ahora, tendr  que correr la voz por los medios informales 
que pueda, pidiendo a los usuarios que "desactiven" la clave p£blica en 
sus ficheros.

Otros usuarios pueden desactivar la clave p£blica en sus propios 
ficheros con la orden "-kd". Si se indica un identificador que no 
corresponde a una clave secreta del fichero correspondiente, la orden 
- -kd lo busca en el fichero de claves p£blicas, y marca esa clave como 
desactivada. Una clave desactivada no puede utilizarse para encriptar 
mensajes, no se puede extraer del fichero con la orden -kx. Puede 
utilizarse para comprobar firmas, pero se muestra una advertencia. 
Adem s, si el usuario intenta a¤adir otra vez la misma clave al fichero, 
no podr , ya que la clave desactivada ya se encuentra en ‚l. Estas 
caracter¡sticas combinadas ayudar n a atajar la difusi¢n de una clave 
desactivada.

Si la clave p£blica indicada ya est  desactivada, la orden -kd le pedir  
si quiere volver a activarla.



Temas avanzados
===============

La mayor¡a de los "Temas avanzados" se recogen en la "Gu¡a del Usuario 
de PGP, Volumen II: Temas especiales". Sin embargo, hay algunos que 
merecen mencionarse aqu¡.


Env¡o de texto cifrado por canales de correo-E: formato Radix-64
=---------------------------------------------------------------

Muchos sistemas de correo electr¢nico s¢lo permiten utilizar mensajes en 
texto ASCII, no con esos datos b sicos binarios de 8 bits que forman el 
texto cifrado. Para solucionar este problema, PGP admite el formato 
ASCII radix-64 para mensajes de texto cifrado, parecido al formato 
Privacy-Enhanced Mail (PEM). Este formato especial representa los datos 
binarios s¢lo con caracteres ASCII imprimibles, por lo que sirve para 
transmitir datos binarios por canales de 7 bits, y para enviar datos 
cifrados como texto normal de correo-E. Este formato hace de "armadura 
de transporte", protegiendo el mensaje mientras viaja por distintas 
pasarelas entre sistemas dentro de Internet. Tambi‚n a¤ade un CRC para 
detectar errores de transmisi¢n.

El formato Radix-64 convierte el texto cifrado expandiendo grupos de 3 
bytes binarios de 8 bits en 4 caracteres ASCII imprimibles, por lo que 
el fichero crece un 33%. Esta expansi¢n no es tan grave cuando se 
considera que PGP probablemente ya hab¡a comprimido el fichero m s que 
eso antes de encriptarlo.

Para producir un fichero cifrado en formato ASCII radix-64, a¤ada la 
opci¢n "a" cuando encripte o firme un mensaje, de la siguiente manera:

     pgp -esa mensaje.txt identificador_des

Este ejemplo producir  un fichero cifrado llamado "mensaje.asc", con los 
datos en un formato ASCII radix-64 parecido al de PEM. Puede cargarse 
entonces en un editor de textos por canales de 7 bits para transmitirlo 
como correo-E normal por Internet o cualquier otra red. 

La desencriptaci¢n del mensaje con armadura de transporte radix-64 es 
id‚ntica a cualquier desencriptaci¢n normal. Por ejemplo:

     pgp mensaje

PGP busca autom ticamente el fichero ASCII "mensaje.asc" antes de buscar 
el binario "mensaje.pgp". Reconoce que el fichero est  en formato 
radix-64, y lo convierte en binario antes de procesarlo normalmente, 
creando el fichero ".pgp" como producto secundario. El fichero final de 
salida ser  de texto normal, precisamente como era el "mensaje.txt" 
original.

La mayor¡a de los servicios de correo-E de Internet no permite enviar 
mensajes de m s de 50.000 bytes. Los m s largos deben dividirse en 
partes, para enviarlas por separado. Si el mensaje cifrado es muy 
grande, y se pide el formato radix-64, PGP lo divide autom ticamente en 
partes m s peque¤as para poder enviarlas por correo-E. Las partes se 
convierten en ficheros con extensiones ".as1", ".as2", ".as3", etc. El 
destinatario debe concatenar por orden estos ficheros en un solo fichero 
grande antes de desencriptar. Durante este proceso, PGP ignora cualquier 
texto extra¤o en las cabeceras del correo, fuera de los bloques radix-64 
del mensaje.

Si quiere enviar una clave p£blica a alguien en formato radix-64, a¤ada 
la opci¢n -a al extraer la clave del fichero.

Si ha olvidado indicar la opci¢n -a cuando al crear un fichero cifrado o 
al extraer una clave, puede convertir el binario al formato radix-64 
poniendo solamente la opci¢n -a, sin encriptar. PGP lo convertir  en un 
fichero ".asc".

Si quiere enviar por correo-E un fichero en texto normal firmado pero no 
cifrado, PGP lo convertir  normalmente en una armadura radix-64, 
haci‚ndolo ilegible para el observador humano habitual. Si el mensaje 
original en texto normal no es binario, hay una forma de enviarlo por 
correo-E de manera que la armadura se aplique s¢lo al certificado de 
firma y no al mensaje. As¡ el destinatario podr  leer el mensaje firmado 
con sus propios ojos, sin ayuda de PGP. Por supuesto, PGP har  falta 
para comprobar la firma. Para obtener m s informaci¢n sobre esta 
caracter¡stica, consulte la explicaci¢n del par metro CLEARSIG en la 
secci¢n "Establecimiento de par metros de configuraci¢n: CONFIG.TXT" en 
el volumen de Temas especiales.


Variable de entorno para la trayectoria
=--------------------------------------

PGP utiliza varios ficheros especiales, tales como los ficheros de 
claves "pubring.pgp" y "secring.pgp", el fichero "ranseed.bin" de 
semilla para n£meros aleatorios, el fichero "config.txt" de 
configuraci¢n y el fichero "language.txt" de traducciones a otros 
idiomas. Estos ficheros especiales pueden colocarse en cualquier 
directorio, siempre que se establezca la variable de entorno "PGPPATH" 
con la trayectoria. Por ejemplo, en MSDOS, la orden:

     SET PGPPATH=C:\PGP

har¡a que PGP asumiese que el nombre del fichero de claves p£blicas es 
"C:\PGP\pubring.pgp"... contando, desde luego, con que ese directorio 
exista. Utilice su editor de textos favorito para modificar el fichero 
AUTOEXEC.BAT, de forma que establezca esta variable al arrancar el 
sistema. Si PGPPATH no se ha definido, se asume que esos ficheros 
especiales est n en el directorio actual.


Establecimiento de los par metros de configuraci¢n: CONFIG.TXT
=-------------------------------------------------------------

PGP tiene ciertos par metros que pueden definirse en un fichero especial 
de configuraci¢n llamado "config.txt" (de texto), que se encuentra en el 
directorio apuntado por la variable de entorno PGPPATH. La existencia de 
un fichero de configuraci¢n permite definir diversas banderas y 
par metros de PGP, y evita la molestia de tener que indicarlas cada vez 
en la l¡nea de ¢rdenes.

Con estos par metros de configuraci¢n puede, por ejemplo, controlarse 
d¢nde va a almacenar PGP sus ficheros temporales, puede elegirse el 
idioma en el que PGP muestra los mensajes de diagn¢stico y los 
indicadores, y puede ajustarse el nivel de escepticismo de PGP para 
determinar la validez de una clave seg£n el n£mero de firmas que la 
certifiquen.

Para obtener m s informaci¢n sobre c¢mo establecer estos par metros de 
configuraci¢n, consulte la secci¢n correspondiente de la Gu¡a del 
Usuario de PGP, volumen de Temas especiales.


Puntos vulnerables
=-----------------

Ning£n sistema de seguridad de datos es impenetrable. PGP puede burlarse 
de diversas maneras. Los posibles puntos vulnerables que hay que tener 
en cuenta son, entre otros, el compromiso de la contrase¤a o de la clave 
secreta, la manipulaci¢n de las claves p£blicas, los ficheros que se han 
borrado pero que siguen todav¡a en el disco, los virus y caballos de 
Troya, los fallos en la seguridad f¡sica, las emisiones 
electromagn‚ticas, la exposici¢n en sistemas multiusuario, el an lisis 
de tr fico, y quiz  incluso el criptoan lisis directo.

Para encontrar un an lisis m s detallado de estos temas, consulte la 
secci¢n "Puntos vulnerables" en la Gu¡a del Usuario de PGP, volumen de 
Temas especiales.


Cuidado con los elixires m gicos
================================

Al examinar un programa criptogr fico, uno siempre se pregunta ¨por qu‚ 
deber¡a fiarme de este producto? Incluso si ha estudiado el c¢digo 
fuente usted mismo, no todo el mundo tiene la experiencia necesaria en 
criptograf¡a para juzgar esa seguridad. Hasta siendo un cript¢grafo 
experimentado pueden escaparse debilidades sutiles en los algoritmos.

Cuando estaba en la universidad a principios de los a¤os setenta, dise¤‚ 
lo que cre¡a un excelente esquema de cifrado. Una sencilla corriente de 
n£meros pseudoaleatorios se sumaba a la corriente de texto normal para 
crear el texto cifrado. As¡ parec¡a frustrarse cualquier an lisis de 
frecuencia del texto cifrado, resultando imposible de romper incluso 
para los servicios de informaci¢n con mayores recursos. Me sent¡a muy 
orgulloso de m¡ mismo por la proeza. Muy presuntuoso.

A¤os despu‚s, descubr¡ ese mismo esquema en varios textos introductorios 
y otros art¡culos sobre criptograf¡a. Qu‚ bien. Otros cript¢grafos 
hab¡an pensado el mismo esquema. Desafortunadamente, se presentaba como 
un sencillo ejercicio sobre c¢mo emplear t‚cnicas criptoanal¡ticas 
elementales para romperlo de forma trivial. Nada m s que decir sobre mi 
excelente esquema.

De esta humillante experiencia aprend¡ qu‚ f cil es caer en un falso 
sentimiento de seguridad cuando se dise¤a un algoritmo de cifrado. La 
mayor¡a de la gente no se da cuenta de lo diab¢licamente dif¡cil que 
resulta dise¤ar un algoritmo que pueda soportar un ataque prolongado y 
porfiado por parte de un oponente con recursos. Muchos ingenieros de 
programaci¢n sin especializar han desarrollado esquemas igualmente 
ingenuos (a menudo exactamente el mismo), y algunos de ellos lo han 
incorporado en programas comerciales de cifrado, vendidos por una buena 
cantidad de dinero a miles de usuarios confiados.

Es algo parecido a vender cinturones de seguridad que tienen buen 
aspecto y parecen buenos, pero que saltan incluso en la prueba de choque 
m s lenta. Dependiendo del caso, pueden resultar incluso peor que no 
llevar ning£n cintur¢n de seguridad en absoluto. Nadie sospecha que son 
malos hasta que hay un choque real. Depender de programas criptogr ficos 
d‚biles puede hacer que usted ponga en peligro informaci¢n confidencial 
sin saberlo. Podr¡a no haber llegado a ocurrir nunca si no hubiese 
tenido ning£n programa criptogr fico. Quiz  usted nunca llegue a 
descubrir que esos datos se han visto comprometidos.

Algunas veces, los programas comerciales utilizan el Federal Data 
Encryption Standard (DES) {Est ndar Federal de Encriptaci¢n de Datos}, 
un buen algoritmo convencional recomendado por el gobierno para uso 
comercial (pero no para informaci¢n clasificada, extra¤amente-- ejem). 
DES puede utilizar varios "modos de funcionamiento", unos mejores que 
otros. El Gobierno recomienda espec¡ficamente no utilizar para los 
mensajes el modo m s simple y d‚bil, Electronic Codebook (ECB) {Libro 
electr¢nico de c¢digos}. Pero s¡ recomienda los modos m s complejos y 
resistentes, Cipher Feedback (CFB) {Retroalimentaci¢n de cifrado} y 
Cipher Block Chaining (CBC) {Encadenamiento de bloques de cifrado}.

Desafortunadamente, la mayor¡a de los programas comerciales de 
encriptaci¢n que he visto utiliza el modo ECB. Cuando he hablado con los 
autores de algunas de esas implementaciones, me han dicho que no hab¡an 
o¡do nunca hablar de los modos CBC o CFB, y que no sab¡an nada de las 
debilidades del modo ECB. El solo hecho de que no hayan aprendido 
suficiente criptograf¡a para conocer esos conceptos elementales no es 
precisamente tranquilizador. Estos mismos programas incluyen a menudo un 
segundo algoritmo exclusivo, m s r pido, que puede utilizarse en lugar 
del DES, que es m s lento. El autor del programa suele pensar que su 
propio algoritmo, m s r pido, es tan seguro como DES, pero despu‚s de 
preguntar suelo descubrir que es s¢lo una variaci¢n de mi propio gran 
esquema de los tiempos de universidad. Puede que ni siquiera est‚ 
dispuesto a revelar c¢mo funciona ese esquema propio, pero me asegura 
que es excelente y que deber¡a confiar en ‚l. Estoy seguro de que cree 
que su algoritmo es excelente, pero ¨c¢mo puedo saberlo sin verlo?

Para ser justo, debo se¤alar que en la mayor¡a de los casos esos 
productos no eran de empresas especializadas en tecnolog¡a 
criptogr fica.

Hay una empresa llamada AccessData (87 East 600 South, Orem, Utah 84058, 
tel‚fono 1-800-658-5199, USA) que vende un programa por $185 que rompe 
los esquemas de cifrado incorporados en WordPerfect, Lotus 1-2-3, MS 
Excel, Symphony, Quattro Pro, Paradox y MS Word 2.0. No s¢lo adivina 
contrase¤as-- hace verdadero criptoan lisis. Algunas personas lo compran 
cuando olvidan la contrase¤a de sus propios ficheros. Las agencias de la 
ley tambi‚n lo compran para poder leer los ficheros que interceptan. 
Habl‚ con Eric Thompson, su autor; me dijo que el programa tarda una 
fracci¢n de segundo en romperlos, y que puso algunos bucles internos 
para retrasarlo y que a sus clientes no les pareciese que resultaba tan 
f cil. Tambi‚n me dijo que el cifrado por contrase¤a de los ficheros 
PKZIP es f cil de romper, y que sus clientes oficiales a menudo 
solicitan ese servicio a otra empresa.

En cierta manera, la criptograf¡a es como los productos farmac‚uticos. 
Su integridad puede ser absolutamente crucial. La mala penicilina tiene 
el mismo aspecto que la buena. Usted puede saber que su hoja de c lculo 
se equivoca, pero ¨c¢mo puede saber si su programa de criptograf¡a es 
d‚bil? El texto cifrado que produce un algoritmo d‚bil tiene tan buen 
aspecto como el texto producido por un algoritmo resistente. Hay mucho 
elixir m gico por ah¡. Muchos remedios de curandero. Al contrario que 
los palmarios buhoneros de medicinas de anta¤o, estos t‚cnicos de 
programaci¢n ni siquiera suelen saber que su producto es un elixir 
m gico. Pueden ser buenos ingenieros de programaci¢n, pero en general no 
han le¡do ninguna publicaci¢n acad‚mica sobre criptograf¡a. Pero creen 
que pueden escribir buenos programas criptogr ficos. ¨Por qu‚ no?  
Despu‚s de todo, parece f cil a primera vista. Y su programa parece 
funcionar bien.

Cualquiera que crea que ha dise¤ado un esquema de encriptaci¢n 
invulnerable puede ser un genio muy poco com£n, o un ingenuo y un 
inexperto.

Recuerdo una conversaci¢n con Brian Snow, cript¢grafo senior de alto 
nivel en la NSA {National Security Agency - Agencia Nacional de 
Seguridad}. Dijo que nunca se fiar¡a de un algoritmo de cifrado dise¤ado 
por alguien que no se hubiese "ganado sus galones"ø pasando mucho tiempo 
rompiendo c¢digos. Tiene mucho sentido. Pude comprobar que pr cticamente 
nadie en el mundo de la criptograf¡a comercial estaba cualificado seg£n 
ese criterio. "S¡", dijo con una sonrisa de seguridad, "Eso hace nuestro 
trabajo en NSA mucho m s f cil". Un pensamiento escalofriante. Yo 
tampoco estoy cualificado.

El Gobierno tambi‚n ha ido vendiendo elixires m gicos. Despu‚s de la 
Segunda Guerra Mundial, los EEUU vendieron m quinas de cifrado Enigma 
alemanas a gobiernos del tercer mundo. Pero no les dijeron que los 
aliados hab¡an roto el c¢digo de Enigma durante la guerra, un hecho que 
permaneci¢ clasificado durante muchos a¤os. Hasta hoy en d¡a, muchos 
sistemas Unix en todo el mundo utilizan el cifrado Enigma para encriptar 
ficheros, en parte porque el Gobierno ha creado obst culos legales 
contra la utilizaci¢n de algoritmos mejores. Incluso intent¢ evitar la 
publicaci¢n inicial del algoritmo RSA en 1977. Adem s, ha machacado 
b sicamente todos los esfuerzos comerciales para desarrollar tel‚fonos 
seguros y efectivos para el p£blico en general.

El trabajo principal de la National Security Agency del Gobierno de los 
EEUU es recoger informaci¢n, especialmente interviniendo encubiertamente 
las comunicaciones privadas de las personas (v‚ase el libro de James 
Bamford "The Puzzle Palace"). La NSA ha acumulado una considerable 
cantidad de habilidad y de medios para romper c¢digos. Si la gente no 
puede conseguir criptograf¡a de calidad para protegerse, el trabajo de 
la NSA se vuelve mucho m s f cil. La NSA tambi‚n tiene la 
responsabilidad de aprobar y recomendar algoritmos de cifrado. Algunos 
cr¡ticos alegan que eso constituye un conflicto de intereses, como poner 
a la zorra a cuidar las gallinas. La NSA ha estado promocionando un 
algoritmo de cifrado convencional dise¤ado por ellos mismos, pero no 
dice a nadie c¢mo funciona porque eso es informaci¢n clasificada. Quiere 
que los dem s se f¡en y lo utilicen. Pero cualquier cript¢grafo puede 
decirle que un algoritmo de cifrado bien dise¤ado no tiene que 
permanecer clasificado para ser seguro. S¢lo las claves deber¡an 
necesitar protecci¢n. ¨C¢mo puede alguien saber realmente si el 
algoritmo clasificado de la NSA es seguro?  No resultar¡a tan dif¡cil 
para la NSA dise¤ar un algoritmo que s¢lo ellos pudiesen romper, si 
nadie m s pudiese revisarlo. ¨Est n vendiendo un elixir m gico 
deliberadamente?

No estoy tan seguro de la seguridad de PGP como lo estaba de mi 
excelente programa de la universidad. Si lo estuviese, ser¡a mala se¤al. 
Sin embargo, estoy bastante seguro de que PGP no contiene ninguna 
debilidad manifiesta. Los criptoalgoritmos fueron desarrollados por 
personas de alto nivel en c¡rculos acad‚micos criptogr ficos civiles, y 
han soportado revisi¢n individualizada por parte de otros expertos. El 
c¢digo fuente de PGP est  disponible para facilitar ese tipo de 
revisi¢n, y para ayudar a disipar los temores de algunos usuarios. Se ha 
investigado razonablemente bien, y ha llevado a¤os escribirlo. Adem s, 
yo no trabajo para la NSA. Espero que no haga falta demasiada "fe"ø para 
confiar en la seguridad de PGP.



Referencia r pida para PGP
==========================

He aqu¡ un r pido resumen de las ¢rdenes de PGP.

Para encriptar un fichero normal con la clave p£blica del destinatario:
     pgp -e fnormal identificador_des

Para firmar un fichero normal con su clave secreta:
     pgp -s fnormal [-u su_identificador]

Para firmar un fichero normal con su clave secreta y despu‚s encriptarlo 
con la clave p£blica del destinatario:
     pgp -es fnormal identificador_des [-u su_identificador]

Para encriptar un fichero normal s¢lo con criptograf¡a convencional, 
escriba:
     pgp -c fnormal

Para desencriptar un fichero, o comprobar la integridad de la firma en 
un fichero firmado:
     pgp fcifrado [-o fnormal]

Para encriptar un mensaje para cualquier n£mero de m£ltiples 
destinatarios:
     pgp -e ftexto identificador1 identificador2 ...

- --- Funciones para la gesti¢n de claves:

Para generar su propio par £nico de claves p£blica/secreta:
     pgp -kg

Para a¤adir el contenido de un fichero de claves p£blicas o secretas 
al anillo correspondiente:
     pgp -ka fdclaves [anillo]

Para extraer (copiar) una clave del anillo de claves p£blicas o 
secretas:
     pgp -kx identificador fdclaves [anillo]
o:   pgp -kxa identificador fdclaves [anillo]

Para ver el contenido del anillo de claves p£blicas:
     pgp -kv[v] [identificador] [anillo]

Para ver la "huella dactilar" de una clave p£blica, y poder 
verificarla por tel‚fono con su due¤o:
     pgp -kvc [identificador] [anillo]

Para ver el contenido y comprobar las firmas de certificaci¢n en el 
anillo de claves p£blicas:
     pgp -kc [identificador] [anillo]

Para modificar el identificador o la frase de contrase¤a de la clave 
secreta:
     pgp -ke identificador [anillo]

Para modificar los par metros de confianza de una clave p£blica:
     pgp -ke identificador [anillo]

Para suprimir una clave, o s¢lo un identificador, del anillo de claves 
p£blicas:
     pgp -kr identificador [anillo]

Para firmar la clave p£blica de alguien en el anillo de claves p£blicas:
     pgp -ks identificador_des [-u su_identificador] [anillo]

Para suprimir de un anillo ciertas firmas de un usuario:
     pgp -krs identificador [anillo]

Para revocar permanentemente su propia clave, emitiendo un certificado 
de compromiso de clave:
     pgp -kd su_identificador

Para desactivar o volver a activar una clave p£blica en el anillo de 
claves p£blicas:
     pgp -kd su_identificador


- --- Ordenes esot‚ricas:

Para desencriptar un mensaje y dejar su firma intacta:
     pgp -d fcifrado

Para crear un certificado de firma separado del documento:
     pgp -sb fnormal [-u su_identificador]

Para separar un certificado de firma del mensaje firmado:
     pgp -b fcifrado

- --- Opciones que pueden utilizarse en combinaci¢n con otras ¢rdenes 
(formando a veces curiosas palabras):

Para producir un fichero cifrado en formato ASCII radix-64, a¤ada 
simplemente la opci¢n -a cuando encripte o firme un mensaje, o 
cuando extraiga una clave:
     pgp -sea fnormal identificador_des
o:   pgp -kax identificador fdclave [anillo]

Para eliminar autom ticamente el fichero normal despu‚s de crear el 
fichero cifrado, a¤ada simplemente la opci¢n -w (wipe {limpiar}) cuando 
encripte o firme un mensaje:
     pgp -wes mensaje.txt identificador_des

Para indicar que un fichero normal contiene texto ASCII, no binario, y 
que debe ajustarse a los convenios de l¡nea del destinatario, a¤ada la 
opci¢n -t (texto) a las otras:
     pgp -seta mensaje.txt identificador_des

Para ver el texto descifrado en la pantalla (como con la orden "more" 
{m s} de Unix), sin grabarlo en un fichero, utilice la opci¢n -m (more) 
al desencriptar:
     pgp -m fcifrado

Para indicar que el texto descifrado para el destinatario SOLO puede 
mostrarse en su pantalla de esta, sin grabarse en el disco, a¤ada la 
opci¢n -m:
     pgp -metas mensaje.txt identificador_des

Para recuperar el nombre original del fichero normal al desencriptar, 
a¤ada la opci¢n -p:
     pgp -p fcifrado

Para utilizar en modo filtro, como en Unix, leyendo de la entrada 
est ndar y escribiendo en la salida est ndar, a¤ada la opci¢n -f:
     pgp -festa identificador_des <fentrada >fsalida



Consideraciones legales
=======================

Para mayor informaci¢n sobre licencias, distribuci¢n, copyrights, 
patentes, marcas registradas, limitaciones de responsabilidad y 
controles de exportaci¢n de PGP, v‚ase la secci¢n "Consideraciones 
legales" de la "Gu¡a del usuario de PGP, volumen II: Temas especiales".

PGP utiliza un algoritmo de clave p£blica reclamado por la patente de 
EEUU n§ 4.405.829. Los derechos exclusivos sobre esta patente est n en 
poder de una compa¤¡a de California llamada Public Key Partners, y usted 
puede estar infringiendo esta patente si utiliza PGP en EEUU. Todo esto 
se explica en el volumen II.

PGP es un programa libre, estilo "guerrilla", gratuito, y no me importa 
que se distribuya ampliamente. Simplemente, no me pida que le env¡e una 
copia. En lugar de eso, puede obtenerla usted mismo en muchas BBS y en 
algunos establecimientos FTP de Internet.


Agradecimientos
===============

Me gustar¡a agradecer a las siguientes personas su contribuci¢n a la 
creaci¢n de Pretty Good Privacy ("intimidad bastante buena"). Aunque fui 
el autor de la versi¢n 1.0 de PGP, grandes partes de las versiones 
siguientes se implementaron gracias a un esfuerzo de colaboraci¢n 
internacional que incluye a un gran n£mero de personas, bajo mi gu¡a en 
el dise¤o.

Branko Lankester, Hal Finney y Peter Gutmann contribuyeron con una 
enorme cantidad de tiempo a a¤adir opciones para PGP 2.0, y lo adaptaron 
a diversas variaciones de Unix. Hal y Branko hicieron esfuerzos 
herc£leos para implementar los nuevos protocolos de gesti¢n de claves. 
Branko le ha dedicado m s tiempo que ning£n otro a PGP.

Hugh Kennedy lo adapt¢ al VAX/VMS, Lutz Frank al Atari ST, y Cor Bosman 
y Colin Plumb al Commodore Amiga.

La traducci¢n de PGP a otros idiomas ha sido realizada por Jean-loup 
Gailly en Francia, Armando Ramos en Espa¤a, Felipe Rodriquez Svensson y 
Branko Lankester en los Pa¡ses Bajos, Miguel Gallardo en Espa¤a (PGP 
1.0), Hugh Kennedy y Lutz Frank en Alemania, David Vincenzetti en 
Italia, Harry Bush y Maris Gabalins en Letonia, Zygimantas Cepaitis en 
Lituania, Peter Suchkow y Andrew Chernov en Rusia, y Alexander 
Smishlajev en Esperantujo. Peter Gutmann se ofreci¢ para traducirlo al 
ingl‚s de Nueva Zelanda, pero finalmente decidimos que PGP podr¡a 
arregl rselas con el ingl‚s de EEUU.

Jean-loup Gailly, Mark Adler y Richard B. Wales publicaron el c¢digo 
para la compresi¢n ZIP, y han dado permiso para incluirlo en PGP. Las 
rutinas MD5 fueron desarrolladas y puestas en el dominio p£blico por Ron 
Rivest. El cifrado IDEA(mr) fue desarrollado por Xuejia Lai y James L. 
Massey en ETH, Zurich, y se utiliza en PGP con permiso de Ascom-Tech AG.

Charlie Merritt fue el primero en ense¤arme c¢mo hacer aritm‚tica 
decente de precisi¢n m£ltiple para la criptograf¡a de clave p£blica, y 
Jimmy Upton contribuy¢ con un algoritmo m s r pido de 
multiplicaci¢n/m¢dulo. Thad Smith implement¢ un algoritmo modmult a£n 
m s r pido. Zhahai Stewart contribuy¢ con muchas ideas £tiles sobre los 
formatos de fichero de PGP y otras cosas, incluyendo el tener m s de un 
identificador de usuario por clave. O¡ de Whit Diffie la idea de tener 
referencias. Kelly Goen hizo la mayor parte del trabajo para la 
publicaci¢n electr¢nica de PGP 1.0.

Ha habido diversas contribuciones de programaci¢n por parte de Colin 
Plumb, Derek Atkins y Castor Fu. Otras contribuciones de esfuerzo, ya 
sean programando o de otra manera, han venido de Hugh Miller, Eric 
Hughes, Tim May, Stephan Neuhaus y muchos otros, demasiados para 
acordarme ahora mismo. Se han iniciado dos proyectos para transportar el 
programa a Macintosh, dirigidos por Zbigniew Fiedorwicz y Blair Weiss.

Desde el lanzamiento de PGP 2.0, muchos otros programadores han enviado 
modificaciones y correcciones de errores, as¡ como ajustes para 
transportar a otros ordenadores. Son demasiados para darles las gracias 
individualmente en este momento.

El desarrollo de PGP se ha convertido en un fen¢meno social notable, 
cuyo atractivo pol¡tico £nico ha inspirado el esfuerzo colectivo de un 
n£mero creciente de programadores voluntarios. En ingl‚s hay un cuento 
para ni¤os llamado "Stone Soup" {Sopa de piedra}. Cada vez es m s 
dif¡cil mirar a trav‚s de la sopa para encontrar la piedra en el fondo 
de la olla, la que dej‚ caer para empezar todo esto.


Acerca del autor
================

Philip Zimmermann es un asesor en ingenier¡a de programaci¢n con 19 a¤os 
de experiencia, especializado en sistemas incorporados en tiempo real, 
criptograf¡a, autenticaci¢n y comunicaciones de datos. Su experiencia 
incluye el dise¤o y la implementaci¢n de sistemas de autenticaci¢n para 
redes de informaci¢n financiera, seguridad en redes de datos, protocolos 
de gesti¢n de claves, ejecutivos multitarea en tiempo real incorporados, 
sistemas operativos y redes de  rea local.

Zimmermann tiene versiones a medida de productos de criptograf¡a y 
autenticaci¢n, e implementaciones de clave p£blica como el NIST DSS, 
adem s de servicios de desarrollo de productos a medida. La direcci¢n de 
su asesor¡a es:

Boulder Software Engineering
3021 Eleventh Street
Boulder, Colorado 80304  USA
Tel‚fono 1-303-541-0140 (voz o FAX)  (10:00am - 7:00pm Mountain Time)
Internet:  prz@acm.org



-----BEGIN PGP SIGNATURE-----
Version: 2.3a

iQCVAgUBLRYHhnAkxuOrd59xAQGPVgQAyjjVmmGXFLzExRXMdw6inqPR4MtwL8zW
7EsGzL6SFnt35wW1i5wZDnvPWm2GC3JHuUzIWkAxmbzx2IIcwTEwYRAmzkA9QNn0
W/TbyLOs86a+n6XjbUkhwHP5u0UcvlOyA7aqQamsXUhJ8sU8TKgeSk48e1T5sGwh
QW51+8Hfgk0=
=1sDs
-----END PGP SIGNATURE-----

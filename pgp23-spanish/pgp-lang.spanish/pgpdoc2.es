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
			Gu°a del Usuario de PGP
		      Volumen II: Temas especiales
		      ----------------------------
			 por Philip Zimmermann
		   Revisado el 14 de junio de 1993


		   PGP Versi¢n 2.3 - 13 junio 93
			 Programa escrito por
			  Philip Zimmermann
				 con
	    Branko Lankester, Hal Finney y Peter Gutmann




Sinopsis:  PGP utiliza criptograf°a de clave p£blica para proteger el
correo electr¢nico y los ficheros de datos.  Comun°quese con seguridad
con personas a las que nunca ha visto, sin necesidad de tener canales
seguros para intercambiar claves.  PGP es r†pido y tiene muchas
prestaciones, que incluyen una completa gesti¢n de claves, firmas
digitales, compresi¢n de datos y un buen dise§o ergon¢mico.


Programa y documentaci¢n (c) Copyright 1990-1993 Philip Zimmermann.
Para mayor informaci¢n sobre licencias, distribuci¢n, copyright,
patentes, marcas registradas, limitaciones de responsabilidad y
controles de exportaci¢n de PGP, vÇase la secci¢n "Consideraciones
legales".

(¯ indica que hay una nota del traductor en el fichero NOTAS.TRA)


Indice
======

Descripci¢n general
Temas especiales
  Elecci¢n de claves por su propio identificador
  Separaci¢n de las firmas de los mensajes
  Desencriptaci¢n del mensaje dejando la firma
  Env°o de ficheros de texto ASCII entre distintos entornos
  Eliminaci¢n de los restos del texto normal en el disco
  Visualizaci¢n del texto descifrado en la pantalla
  C¢mo hacer un mensaje "S¢lo para sus ojos"
  Conservaci¢n del nombre original del fichero de texto normal
  Modificaci¢n del identificador de usuario o de la frase de contrase§a
  Modificaci¢n de los par†metros de confianza de una clave p£blica
  Comprobaci¢n de que todo est† bien en el fichero de claves p£blicas
  Verificaci¢n de una clave p£blica por telÇfono
  Utilizaci¢n de PGP como un filtro UNIX
  Supresi¢n de las preguntas innecesarias:  BATCHMODE
  Respuesta "S°" obligada a las preguntas de confirmaci¢n:  FORCE
  PGP devuelve a la coraza ("shell") el estado de salida
  Variable de entorno para la frase de contrase§a
  Establecimiento de los par†metros de configuraci¢n: CONFIG.TXT
    TMP - Trayectoria para ficheros temporales
    LANGUAGE - Elecci¢n del idioma
    MYNAME - Identificador de usuario por omisi¢n para firmar
    TEXTMODE - Asumir que el texto normal es un fichero de texto
    CHARSET - Indica el conjunto local de caracteres
    ARMOR - Activa la salida con armadura ASCII
    ARMORLINES - Tama§o de ficheros por partes con armadura ASCII
    KEEPBINARY- Conservar ficheros cifrados binarios tras desencriptar
    COMPRESS - Activar compresi¢n
    COMPLETES_NEEDED - Referencias completamente fiables necesarias
    MARGINALS_NEEDED - Referencias relativamente fiables necesarias
    CERT_DEPTH - Profundidad de anidamiento de referencias
    BAKRING - Nombre de la copia de seguridad del fichero secreto
    PAGER - Elegir la orden de coraza para mostrar texto normal
    SHOWPASS - Mostrar al usuario la frase de contrase§a
    TZFIX - Ajuste de zona horaria
    CLEARSIG - Activar Mensajes firmados encapsulados como texto normal
    VERBOSE - Mensajes normales, verbosos o sin mensajes
    INTERACTIVE - Pedir confirmaci¢n para a§adir claves
  Protecci¢n contra falsos sellos de hora
  Una mirada en el interior¯
    N£meros aleatorios
    Algoritmo de cifrado convencional de PGP
    Compresi¢n de datos
    Res£menes de mensajes y firmas digitales
  Compatibilidad con las versiones anteriores de PGP
Puntos vulnerables
  Compromiso de la frase de contrase§a y de la clave secreta
  Manipulaci¢n de claves p£blicas
  Ficheros "no borrados del todo"
  Virus y caballos de Troya
  Fallos en la seguridad f°sica
  Ataques Tempest
  Exposici¢n en sistemas multiusuario
  An†lisis de tr†fico
  Criptoan†lisis
Consideraciones legales
  Marcas registradas, copyrights y garant°as.
  Derechos de patente sobre los algoritmos
  Licencias y distribuci¢n
  Controles de exportaci¢n
Grupos pol°ticos relacionados con los ordenadores
Lecturas recomendadas
C¢mo ponerse en contacto con el autor
ApÇndice A:  D¢nde obtener PGP


Descripci¢n general
===================

Pretty Good(mr) Privacy (PGP), "intimidad¯ bastante buena", de Phil's
Pretty Good Software, es una aplicaci¢n inform†tica de criptograf°a de
alta seguridad en DOS, Unix, VAX/VMS y otros ordenadores. PGP combina la
comodidad del criptosistema de clave p£blica de Rivest-Shamir-Adleman
(RSA) con la velocidad de la criptograf°a convencional, res£menes de
mensajes para firmas digitales, compresi¢n de datos antes de encriptar,
un buen dise§o ergon¢mico y con una completa gesti¢n de claves.

El volumen II de la Gu°a del usuario trata aquellos temas avanzados que
no se inclu°an en el volumen I, "Temas esenciales". Deber°a leer antes
ese primer volumen, o este no le resultar† muy comprensible. La lectura
de este volumen II es opcional.


Temas especiales
================

Elecci¢n de claves por su propio identificador
=---------------------------------------------

En todas las ¢rdenes que permiten escribir un identificador de usuario o
un fragmento para elegir una clave, puede utilizarse su identificador
hexadecimal. S¢lo tiene que indicar ese identificador, con el prefijo
"0x", en lugar del identificador de usuario.
Por ejemplo:

     pgp -kv 0x67F7

Muestra todas las claves que tienen 67F7 como parte de su identificador.

Esta caracter°stica resulta particularmente £til si tiene dos claves
diferentes de la misma persona, con un mismo identificador de usuario.
Puede elegir la clave, sin ambigÅedad, especificando el identificador de
clave.


Separaci¢n de las firmas de los mensajes
=---------------------------------------

Normalmente, los certificados de firma se a§aden f°sicamente al texto al
que se refieren. Resulta c¢modo comprobarlas de esa manera en los casos
sencillos. Bajo ciertas circunstancias, resulta m†s conveniente tener
los certificados aparte del mensaje que firman. Se pueden generar
certificados separados del texto. Para ello, combine la opci¢n 'b'
(break {romper}) con la opci¢n 's' (sign {firmar}). Por ejemplo:

     pgp -sb carta.txt

Se produce un certificado aislado, en un fichero llamado "carta.sig". El
contenido de carta.txt no se a§ade al certificado.

DespuÇs de crear el fichero del certificado de firma (carta.sig en el
ejemplo anterior), env°eselo al destinatario junto con el fichero de
texto. El destinatario debe tener los dos ficheros para poder comprobar
la integridad de la firma. Cuando intente procesar el fichero de firma,
PGP se dar† cuenta de que no hay texto en el fichero, y pedir† un
nombre. S¢lo as° podr† comprobar PGP la integridad de la firma. Si el
destinatario sabe con anterioridad que la firma est† separada del texto,
puede indicar los dos nombres en la l°nea de ¢rdenes:

     pgp carta.sig carta.txt
o:   pgp carta carta.txt

En este caso, PGP no tiene que pedir el nombre del texto.

Los certificados separados sirven para mantenerlo en un registro aparte.
La firma separada de un programa ejecutable podr°a servir para detectar
una infecci¢n por virus. TambiÇn resulta conveniente si varias partes
deben firmar un documento, tal como un contrato legal, sin necesidad de
anidar las firmas. Todas ellas son independientes.

Si recibe un fichero de texto cifrado con certificado de firma, puede
separarlo durante la desencriptaci¢n. Se hace con la opci¢n -b en el
descifrado, como en:

     pgp -b carta

Se desencripta el fichero carta.pgp, y, si hay alguna firma en Çl, PGP
la comprueba y la separa del resto del mensaje, almacen†ndola en el
fichero carta.sig.


Desencriptaci¢n del mensaje dejando la firma
=-------------------------------------------

Normalmente, usted querr† que PGP desenmara§e completamente el texto
cifrado, desencript†ndolo y comprobando la firma anidada si existe,
pelando las capas hasta que s¢lo quede el fichero original en texto
normal.

Sin embargo, quiz† quiera algunas veces desencriptar el fichero dejando
la firma dentro, de forma que quede el fichero descifrado y firmado.
Puede resultar conveniente para enviar la copia firmada de un documento
a una tercera persona, quiz† volviÇndolo a cifrar. Por ejemplo, suponga
que recibe un mensaje firmado por Charlie, cifrado para usted. Querr†
desencriptarlo, y, dejando en Çl la firma de Charlie, envi†rselo a
Alice, quiz† volviÇndolo a cifrar con la clave p£blica de Alice. No hay
problema. PGP puede hacerlo.

Para desencriptar solamente el mensaje, dejando la firma intacta,
escriba:

     pgp -d carta

As° se desencripta carta.pgp, y, si hay una firma dentro, se deja
intacta en el fichero de salida junto al texto normal descifrado.

Entonces puede archivarla, o volverla a encriptar y envi†rsela a
alguien.


Env°o de ficheros de texto ASCII entre distintos entornos
=--------------------------------------------------------

Puede utilizar PGP para encriptar cualquier tipo de fichero normal, ya
contenga datos de 8 bits o texto ASCII. Probablemente, el uso m†s
habitual de PGP ser† para correo-E, donde el texto normal es ASCII.

El texto ASCII se representa de forma diferente en distintos entornos.
Por ejemplo, en DOS todas las l°neas acaban en un retorno de carro
seguido de un salto de l°nea. En UNIX, todas las l°neas terminan s¢lo
con un salto de l°nea. En Macintosh, las l°neas terminan s¢lo con un
retorno de carro. Es una triste realidad.

Los mensajes en texto ASCII se traducen muchas veces autom†ticamente a
una forma "can¢nica" com£n cuando se transmiten de una m†quina a otra.
El texto can¢nico tiene un retorno de carro y un salto de l°nea al final
de cada l°nea de texto. Por ejemplo, el conocido protocolo de
comunicaciones KERMIT puede convertir el texto a forma can¢nica cuando
lo transmite a otro sistema. El KERMIT que recibe convierte las
terminaciones de l°nea a la forma local. As° resulta m†s f†cil compartir
ficheros de texto entre diferentes sistemas.

El protocolo de comunicaciones no puede convertir autom†ticamente el
texto cifrado, ya que el original queda oculto durante el encriptado.
Para solucionar esta incomodidad, PGP le permite indicar que hay que
tratar el texto normal como ASCII (no como datos binarios), y que debe
convertirse en texto can¢nico antes de encriptar. En el lado receptor,
el texto normal descifrado vuelve a convertirse autom†ticamente a la
forma apropiada para el entorno local.

Para hacer que PGP asuma que el texto normal debe convertirse a la forma
can¢nica antes de encriptar, s¢lo hay que a§adir la opci¢n "t" al
encriptar o firmar un mensaje, como en:

     pgp -et mensaje.txt identificador_des

Este modo se desconecta autom†ticamente si PGP detecta algo que
considere un dato binario, no de texto, en el fichero normal.

Para aquellos usuarios de PGP con conjunto de caracteres de 8 bits, PGP,
al pasar el texto a la forma can¢nica, puede convertir los datos del
conjunto local de caracteres al conjunto LATIN1 (ISO 8859-1 Latin
Alphabet 1), dependiendo del par†metro CHARSET del fichero de
configuraci¢n de PGP. LATIN1 es un superconjunto de ASCII, con
caracteres a§adidos para muchos idiomas europeos.


Eliminaci¢n de los restos del texto normal en el disco
=-----------------------------------------------------

DespuÇs de cifrar un fichero, usted puede hacer que PGP sobrescriba
autom†ticamente el texto normal y lo borre sin dejar rastro, para que
nadie pueda recuperarlo m†s adelante con una utilidad de exploraci¢n de
bloques de disco. Resulta £til si el fichero de texto normal contiene
informaci¢n confidencial que usted no quiere tener por ah°.

Para eliminar el fichero normal despuÇs de producir el texto cifrado,
a§ada la opci¢n "w" (wipe {limpiar}) cuando encripte o firme un mensaje,
como en:

     pgp -esw mensaje.txt identificador_des

Este ejemplo crea el fichero cifrado "mensaje.pgp", y el fichero normal
"mensaje.txt" se destruye sin posibilidad de recuperaci¢n.

Naturalmente, debe tener cuidado con esta opci¢n. Tenga tambiÇn en
cuenta que este proceso no elimina ning£n fragmento de texto normal que
el procesador de texto haya creado en el disco mientras se editaba el
mensaje antes de ejecutar PGP. La mayor°a de los procesadores crean
copias de seguridad, ficheros provisionales, o ambas cosas. Por otra
parte, s¢lo se sobrescribe el fichero una vez-- suficiente para frustrar
intentos convencionales de recuperaci¢n del disco, pero no para soportar
un esfuerzo porfiado, con recursos, por recuperar los dÇbiles restos
magnÇticos de los datos, mediante maquinaria especial para recuperar
discos.


Visualizaci¢n del texto descifrado en la pantalla
=------------------------------------------------

Para ver el texto descifrado en la pantalla (como al utilizar una orden
parecida al "more" {"m†s"} de Unix), sin escribirlo en un fichero,
utilice la opci¢n -m (more):

     pgp -m fcifrado

As° se muestra el texto normal descifrado, pantalla a pantalla.


C¢mo hacer un mensaje "S¢lo para sus ojos"
=-----------------------------------------

Para hacer que el texto descifrado SOLO se muestre en la pantalla de la
destinataria, sin poder grabarse en el disco, a§ada la opci¢n -m:

     pgp -mes mensaje.txt identificador_des

M†s adelante, cuando la destinataria desencripte el texto cifrado con su
clave secreta y su contrase§a, el texto normal se mostrar† en la
pantalla, sin grabarse en el disco. El texto aparecer† como si hubiese
utilizado la orden "more" de Unix, pantalla a pantalla. Si quiere leer
el mensaje otra vez, tendr† que desencriptarlo de nuevo.

Esta caracter°stica proporciona el medio m†s seguro para evitar que un
mensaje confidencial quede accidentalmente en el disco de la
destinataria. Se a§adi¢ por petici¢n de una usuaria que quer°a enviar
mensajes °ntimos a su amante, pero que ten°a miedo de dejar
inadvertidamente los mensajes descifrados en el ordenador de su marido.


Conservaci¢n del nombre original del fichero de texto normal
=-----------------------------------------------------------

Normalmente, PGP da al fichero descifrado un nombre parecido al del
texto cifrado, simplemente eliminando la extensi¢n. Puede anularse este
convenio, indicando en la l°nea de ¢rdenes un fichero de salida para el
texto normal, con la opci¢n -o. Para la mayor parte del correo-E es una
buena forma de nombrar el fichero normal, que permite decidirlo en el
momento de descifrar, ya que los mensajes vienen habitualmente de textos
con nombres sin utilidad como "to_phil.txt" {"a_phil.txt"}.

Sin embargo, cuando PGP encripta un fichero normal, siempre guarda el
nombre original y lo a§ade al texto normal antes de comprimirlo y
encriptarlo. Normalmente, PGP desecha este nombre al desencriptar, pero
puede indicarle que lo conserve y que lo utilice en el fichero de salida
descifrado. Resulta £til cuando se aplica PGP a ficheros cuyos nombres
hay que conservar.

Para recuperar el nombre original del texto normal al desencriptar,
a§ada la opci¢n -p, como en:

     pgp -p fcifrado

Normalmente, nunca utilizo esta opci¢n, porque de otro modo la mitad del
correo-E que entra se desencriptar°a en ficheros con los mismos nombres
"to_phil.txt" o "prz.txt".


Modificaci¢n del identificador de usuario o de la frase de contrase§a
=--------------------------------------------------------------------

En ciertos casos, necesitar† cambiar su frase de contrase§a, quiz†
porque hubiese alguien mirando por encima del hombro mientras usted la
estaba escribiendo.

Puede querer tambiÇn cambiar su identificador de usuario, porque se haya
casado o cambiado de nombre, o porque haya cambiado su direcci¢n de
correo-E. A lo mejor quiere a§adir un segundo o tercer identificador de
usuario a la clave, por tener m†s de un nombre o direcci¢n de correo-E,
o por querer indicar m†s de un cargo profesional. PGP permite incluir
m†s de un identificador en la clave, y se puede utilizar cualquiera de
ellos para buscarla en un fichero.

Para modificar el identificador o la frase de contrase§a de su clave
secreta:

     pgp -ke identificador [fdclaves]

PGP pedir† un nuevo identificador o una nueva contrase§a.

El par†metro opcional [fdclaves], si se indica, debe ser un fichero de
claves p£blicas, no secretas. El campo identificador debe ser el suyo
propio, que PGP reconoce porque aparece tanto en el fichero de claves
p£blicas como en el de claves secretas. Ambos ficheros de claves se
actualizan, aunque usted s¢lo indique el fichero de claves p£blicas.


Modificaci¢n de los par†metros de confianza de una clave p£blica
=---------------------------------------------------------------

En ciertas ocasiones har† falta cambiar los par†metros de confianza de
una clave p£blica del fichero. La explicaci¢n del significado de estos
par†metros aparece en la secci¢n "®C¢mo controla PGP la validez de las
claves?" en el volumen sobre Temas esenciales de la Gu°a del usuario de
PGP.

Para modificar los par†metros de confianza de una clave p£blica:

     pgp -ke identificador [fichero]


Comprobaci¢n de que todo est† bien en el fichero de claves p£blicas
=------------------------------------------------------------------

Normalmente, PGP comprueba autom†ticamente cualquier clave o firma nueva
en el fichero de claves p£blicas, y actualiza los par†metros de
confianza y las puntuaciones de validez. En teor°a, mantiene actualizada
toda la informaci¢n sobre la validez de las claves al a§adir o suprimir
material del fichero. Sin embargo, puede hacer falta que PGP realice un
an†lisis exhaustivo del fichero de claves p£blicas, comprobando las
firmas de certificaci¢n y los par†metros de confianza, poniendo al d°a
todas las puntuaciones de validez, y que compruebe su propia clave,
fundamentalmente fiable, compar†ndola con una copia de seguridad en un
disco protegido contra escritura. Es conveniente realizar peri¢dicamente
este mantenimiento higiÇnico, para comprobar que todo va bien en el
fichero de claves p£blicas. Para indicar a PGP que realice un an†lisis
completo del fichero, utilice la opci¢n -kc (keyring check)
{comprobaci¢n del fichero}:

     pgp -kc

TambiÇn puede hacer que PGP compruebe las firmas de una sola clave
p£blica, con:

     pgp -kc identificador [fichero]

Para obtener m†s informaci¢n sobre la comprobaci¢n de la copia de
seguridad de su propia clave, consulte la descripci¢n del par†metro
BAKRING en la secci¢n sobre el fichero de configuraci¢n, en este mismo
manual.


Verificaci¢n de una clave p£blica por telÇfono
=---------------------------------------------

Si obtiene una clave p£blica que no ha sido certificada por nadie en
quien usted conf°e, ®c¢mo puede saber que es realmente su clave?  La
mejor forma de comprobar una clave sin certificar es verificarla por
alg£n canal independiente, distinto del que le sirvi¢ para recibir la
clave. Una buena forma, si conoce a la persona y la podr°a reconocer por
telÇfono, ser°a llamarla y verificar la clave por telÇfono. En lugar de
leer toda la clave (en armadura ASCII) por telÇfono, una tarea pesada,
podr°a leerles s¢lo su "huella dactilar". Para ver esa huella dactilar,
utilice la orden -kvc:

     pgp -kvc identificador [fichero]

De esta manera se mostrar† la clave con un resumen de 16 bytes de los
componentes de la clave p£blica. Lea por telÇfono esta huella dactilar
de 16 bytes a la due§a de la clave, mientras ella la comprueba con la
suya propia, obtenida con la misma orden.

Pueden as° verificarse las claves mutuamente, y firmarlas con confianza.
Es una forma segura y c¢moda de iniciar la red de confianza de las
claves en su c°rculo de amigos.


Utilizaci¢n de PGP como un filtro UNIX
=-------------------------------------

Los aficionados a Unix est†n acostumbrados a los "conductos" (pipes)
para conseguir que dos aplicaciones trabajen conjuntamente. La salida de
una aplicaci¢n se dirige como entrada de otra por medio de un conducto.
Para que funcione, las aplicaciones tienen que poder leer el material en
bruto por la "entrada est†ndar" (standard input), y escribir la salida
procesada en la "salida est†ndar" (standard output). PGP puede funcionar
de este modo. Si no entiende quÇ quiere decir todo esto, seguramente no
lo necesita.

Para utilizar un filtro tipo Unix, leyendo de la entrada est†ndar y
escribiendo en la salida est†ndar, elija la opci¢n -f, como en:

     pgp -festa identificador_des <fentrada >fsalida

Esta caracter°stica hace m†s sencillo adaptar PGP a las aplicaciones de
correo electr¢nico.

Cuando utilice PGP en modo filtro para desencriptar un fichero cifrado,
puede utilizar la variable de entorno PGPPASS para la frase de
contrase§a, y que PGP no la pida. La caracter°stica PGPPASS se explica
m†s adelante.


Supresi¢n de las preguntas innecesarias:  BATCHMODE
=--------------------------------------------------

Con la bandera BATCHMODE (modo por lotes) activada en la l°nea de
¢rdenes, PGP no har† ninguna pregunta innecesaria ni pedir† ficheros
alternativos. He aqu° un ejemplo de c¢mo establecer esta bandera:

     pgp +batchmode fcifrado

Resulta £til para ejecutar PGP de forma no interactiva desde guiones de
Unix o ficheros por lotes DOS. Algunas ¢rdenes de gesti¢n de claves
necesitan intervenci¢n del usuario incluso con BATCHMODE activada, por
lo que los guiones tendr†n que evitarlas.

TambiÇn se puede activar BATCHMODE para comprobar la validez de la firma
de un fichero. Si no hab°a firma en el fichero, el c¢digo de salida es
1. Si ten°a una firma correcta, el c¢digo de salida es 0.


Respuesta "S°" obligada a las preguntas de confirmaci¢n:  FORCE
=--------------------------------------------------------------

Esta bandera de l°nea de ¢rdenes hace que PGP asuma "s°" como respuesta
del usuario a las preguntas de confirmaci¢n para sobreescribir un
fichero existente, o para eliminar una clave del fichero por medio de la
orden -kr. He aqu° un ejemplo de c¢mo establecer esta bandera:

     pgp +force fcifrado
o:
     pgp -kr +force Smith

Esta caracter°stica resulta £til al ejecutar PGP de forma no interactiva
desde un gui¢n Unix o fichero por lotes DOS.


PGP devuelve a la coraza ("shell") el estado de salida
=-----------------------------------------------------

Para facilitar la ejecuci¢n de PGP en modo "por lotes", como desde
ficheros ".bat" de DOS o guiones de Unix, PGP devuelve el estado de
error de salida a la coraza ("shell"). Un c¢digo igual a cero indica una
salida normal, y un c¢digo distinto de cero indica que ha habido alg£n
error. Las diferentes condiciones de error devuelven a la coraza
distintos c¢digos.


Variable de entorno para la frase de contrase§a
=----------------------------------------------

Normalmente, PGP pide al usuario que escriba una frase de contrase§a
cada vez que se necesita para abrir una clave secreta. Sin embargo,
puede almacenarse esa frase en una variable de entorno, desde la coraza
de ¢rdenes del sistema operativo. La variable PGPPASS contiene la frase
de contrase§a que PGP intentar† utilizar primero. Si es incorrecta, PGP
resolver† la situaci¢n pidiendo la frase correcta.

Por ejemplo, en DOS, la orden de coraza:

     SET PGPPASS=zaphod beeblebrox for president

eliminar† el indicador de la frase de contrase§a si era "zaphod
beeblebrox for president".

Esta peligrosa caracter°stica facilita las cosas si tiene que tratar
habitualmente con muchos mensajes dirigidos a su clave secreta,
eliminando la necesidad de repetir la contrase§a cada vez que ejecuta
PGP.

He a§adido esta caracter°stica por petici¢n popular. No obstante, es
bastante peligrosa, ya que deja su valiosa contrase§a almacenada en un
lugar distinto de su propio cerebro. A£n peor, si es usted
particularmente imprudente, podr°a estar incluso en el disco del mismo
ordenador que tiene la clave secreta. Ser°a especialmente absurdo y
peligroso instalar esta orden en un fichero por lotes o de gui¢n, como
el AUTOEXEC.BAT de DOS. Cualquiera podr°a aparecer durante la hora de la
comida y robar tanto el fichero de claves secretas como el fichero con
la frase de contrase§a.

No hay forma de insistir lo suficiente sobre la importancia de este
riesgo. Si va a utilizar esta caracter°stica, aseg£rese de leer las
secciones "Exposici¢n en sistemas multiusuario" y "C¢mo proteger las
claves secretas contra revelaci¢n" en este volumen y en el volumen sobre
Temas esenciales de la Gu°a del Usuario de PGP.

Si tiene que utilizarla, el mÇtodo m†s seguro ser°a escribir a mano la
orden, para establecer PGPPASS cada vez que arranque la m†quina para
utilizar PGP, y borrar la variable o apagar el ordenador cuando haya
terminado. Nunca, definitivamente, debe hacerlo en un entorno donde
alguien m†s pueda acceder a su m†quina. Esa persona podr°a llegar y
simplemente pedir al ordenador que mostrase el contenido de PGPPASS.


Establecimiento de los par†metros de configuraci¢n: CONFIG.TXT
==============================================================

PGP tiene diversos par†metros ajustables, que pueden definirse en un
fichero de texto especial, llamado "config.txt", en el directorio
se§alado por la variable de entorno PGPPATH. El fichero de configuraci¢n
permite al usuario definir diversas banderas y par†metros, sin tener que
hacerlo siempre en la l°nea de ¢rdenes.

Los par†metros de configuraci¢n pueden contener valores enteros, cadenas
de caracteres, o valores "on/off" {encendido/apagado}, seg£n el tipo de
par†metro. Se proporciona un fichero de muestra con PGP, para que pueda
ver algunos ejemplos.

En el fichero de configuraci¢n se ignoran las l°neas en blanco, al igual
que cualquier cosa que vaya despuÇs del car†cter de comentario "#". Las
palabras clave no tienen en cuenta el tipo de letra (may£sculas o
min£sculas).

He aqu° un fragmento de muestra de un fichero de configuraci¢n t°pico

# TMP es el directorio de los ficheros de borrador de PGP, p.ej.
# un disco en RAM
TMP = "e:\"  # La variable TMP del entorno puede imponerse.
Armor = on   # Se pone -a para armadura ASCII donde corresponda.
# CERT_DEPTH indica con quÇ profundidad las referencias pueden
# servir como referencias de otras.
cert_depth = 3

Si no se definen algunos par†metros en el fichero de configuraci¢n, o no
hay tal fichero, o PGP no puede encontrarlo, se eligen por omisi¢n
algunos valores razonables.

Tenga en cuenta que tambiÇn se pueden establecer esos par†metros de
configuraci¢n directamente desde la l°nea de ¢rdenes de PGP, precediendo
el valor con un car†cter "+". Por ejemplo, estas dos ¢rdenes PGP
producen el mismo efecto:

    pgp -e +armor=on mensaje.txt burua
o:  pgp -ea mensaje.txt burua


A continuaci¢n, se indican los diversos par†metros que pueden definirse
en el fichero de configuraci¢n.

TMP - Trayectoria para ficheros temporales
=-----------------------------------------

Valor por omisi¢n:  TMP = ""

El par†metro TMP indica en quÇ directorio se sit£an los ficheros
temporales que necesita PGP. El mejor sitio para ponerlos es un disco
RAM, si lo tiene. De esa manera se aceleran bastante las cosas, y
aumenta la seguridad. Si TMP no est† definido, los ficheros temporales
se sit£an en el directorio actual. Si se ha definido la variable de
entorno TMP, PGP la utiliza para poner los ficheros temporales.


LANGUAGE - Elecci¢n del idioma
=-----------------------------

Valor por omisi¢n:  LANGUAGE = "en"

PGP muestra en la pantalla diversos indicadores, mensajes de advertencia
y consejos para el usuario. Por ejemplo, mensajes como "No se encuentra
el fichero" o "Introduzca su contrase§a:". Estos mensajes est†n
normalmente en inglÇs. Sin embargo, puede hacer que PGP muestre sus
mensajes en otros idiomas, sin tener que modificar el programa
ejecutable.

Personas de diversos pa°ses han traducido todos los mensajes,
advertencias e indicadores de PGP a sus idiomas maternos. Estos cientos
de mensajes traducidos se encuentran en un fichero especial de texto,
llamado "language.txt", que se distribuye con PGP. Los mensajes de este
fichero se pueden encontrar en inglÇs, castellano, neerlandÇs, alem†n,
francÇs, italiano, ruso, let¢n y lituano. Podr†n a§adirse otros idiomas
m†s adelante.

El par†metro LANGUAGE indica en quÇ idioma se muestran los mensajes.
LANGUAGE puede ser "en" para inglÇs, "es" para castellano, "de" para
alem†n, "nl" para neerlandÇs, "fr" para francÇs, "it" para italiano,
"ru" para ruso, "lt3" para lituano, "lv" para let¢n o "esp" para
esperanto. Por ejemplo, si aparece la siguiente l°nea en el fichero de
configuraci¢n:

LANGUAGE = "es"

PGP elegir°a el castellano para mostrar los mensajes. La elecci¢n por
omisi¢n es inglÇs.

Cuando PGP tiene que mostrar un mensaje, busca en el fichero
"language.txt" el equivalente en el idioma elegido, y lo presenta. Si no
puede encontrar el fichero de idiomas, si el idioma elegido no se
encuentra en Çl o si alguna frase no se encuentra traducida o no est† en
el fichero, PGP mostrar† el mensaje en inglÇs.

Para ahorrar espacio en el disco, no se incluye la mayor parte de las
traducciones en el lanzamiento est†ndar de PGP, pero se pueden obtener
por separado.


MYNAME - identificador de usuario por omisi¢n para firmar
=--------------------------------------------------------

Valor por omisi¢n:  MYNAME = ""

El par†metro de configuraci¢n MYNAME indica quÇ identificador de usuario
por omisi¢n sirve para elegir la clave secreta que se utiliza para
firmar. Si MYNAME no est† definido, se utiliza la clave secreta m†s
reciente del fichero. Puede anularse este valor, indicando un
identificador de usuario en la l°nea de ¢rdenes de PGP con la opci¢n -u.


TEXTMODE - Asumir que el texto normal es un fichero de texto
=-----------------------------------------------------------

Valor por omisi¢n:  TEXTMODE = off

El par†metro TEXTMODE es equivalente a la opci¢n -t de la l°nea de
¢rdenes. Si est† activado, hace que PGP asuma que el texto normal es un
fichero de texto, en lugar de un fichero binario, y lo convierta en
"texto can¢nico" antes de encriptarlo. El texto can¢nico tiene un
retorno de carro y un salto de l°nea al final de cada l°nea de texto.

Este modo se desactiva autom†ticamente si PGP detecta en el fichero algo
que considera un dato binario.

En sistemas VAX/VMS, la versi¢n actual de PGP tiene como elecci¢n por
omisi¢n TEXTMODE=ON.

Para obtener m†s informaci¢n, consulte la secci¢n "Env°o de ficheros de
texto ASCII entre distintos entornos".


CHARSET - Indica el conjunto local de caracteres
=-----------------------------------------------

Valor por omisi¢n:  CHARSET = NOCONV

Dado que PGP tiene que procesar los mensajes en muchos idiomas
diferentes, con conjuntos de caracteres distintos de ASCII, puede hacer
falta indicar a PGP quÇ conjunto  de caracteres utiliza su m†quina. As°
se determina que conversi¢n hay que realizar al convertir ficheros
normales de texto desde y hacia formato can¢nico. S¢lo tiene relevancia
si usted se encuentra en un entorno sin inglÇs ni ASCII.

El par†metro CHARSET elige el conjunto local de caracteres. Las
posibilidades son NOCONV (no convertir), LATIN1 (ISO 8859-1 Lat°n
Alphabet 1), KOI8 (utilizado en la mayor°a de los sistemas Unix en
Rusia), ALT-CODES (utilizado en sistemas rusos DOS) y CP850 (utilizado
en la mayor°a de los idiomas de Europa Occidental en PCs est†ndar DOS).

LATIN1 es la representaci¢n interna que emplea PGP para texto can¢nico,
por lo que al elegir LATIN1 no se realiza ninguna conversi¢n. Tenga
tambiÇn en cuenta que PGP trata KOI8 como LATIN1 aunque sea un conjunto
de caracteres completamente diferente (ruso), ya que tratar de convertir
KOI8 a LATIN1 o CP850 ser°a in£til. Esto quiere decir que hacer CHARSET
igual a NOCONV, LATIN1 o KOI8 resulta equivalente para PGP.

Si utiliza DOS y espera enviar y recibir informaci¢n en idiomas de
Europa Occidental, establezca CHARSET = "CP850". As° se har† que PGP
convierta los mensajes can¢nicos entrantes de LATIN1 a CP850 despuÇs de
desencriptar. Si utiliza la opci¢n -t (texto) para convertir a texto
can¢nico, PGP convierte el texto CP850 a LATIN1 antes de encriptarlo.

Para mayor informaci¢n, vÇase la secci¢n "Env°o de ficheros de texto
ASCII entre distintos entornos".


ARMOR - Activa la salida con armadura ASCII
=------------------------------------------

Valor por omisi¢n:  ARMOR = off

El par†metro ARMOR es equivalente a la opci¢n -a en la l°nea de ¢rdenes.
Si est† activado, hace que PGP produzca el texto cifrado y las claves en
formato ASCII Radix-64, apropiado para enviar por canales de correo-E.
Los ficheros de salida reciben la extensi¢n ".asc".

Si utiliza PGP principalmente para correo-E, puede resultar conveniente
activar este par†metro.

Para obtener m†s informaci¢n, consulte la secci¢n "Env°o de texto
cifrado por canales de correo-E: formato Radix-64" en el volumen sobre
Temas esenciales.


ARMORLINES - Tama§o de ficheros por partes con armadura ASCII
=------------------------------------------------------------

Valor por omisi¢n:  ARMORLINES = 720

Cuando PGP crea un fichero ".asc" muy grande en radix-64, para enviar el
texto cifrado o las claves por correo-E, lo divide en trozos separados
lo bastante peque§os como para poder enviarlos con las utilidades para
correo de Internet. Normalmente, los procesadores de correo en Internet
proh°ben ficheros de m†s de unos 50.000 bytes, por lo que, restringiendo
el n£mero de l°neas a aproximadamente 720, nos mantendremos
tranquilamente dentro de los l°mites. Las partes del fichero toman como
sufijos ".as1", ".as2", ".as3", ...

El par†metro de configuraci¢n ARMORLINES indica el m†ximo n£mero de
l°neas para cada parte de la secuencia de ficheros ".asc". Si se hace
cero, PGP no divide el fichero.

Los ficheros de correo en Fidonet tienen un l°mite superior de unos 32K
bytes, por lo que 450 l°neas ser°a m†s apropiado en esos entornos {de
todos modos, parece que hay una "policy" que proh°be el env°o de
ficheros cifrados por "netmail").

Para obtener m†s informaci¢n, consulte la secci¢n "Env°o de texto
cifrado por canales de correo-E: formato Radix-64" en el volumen sobre
Temas esenciales.


KEEPBINARY - Conservar ficheros cifrados binarios tras desencriptar
=------------------------------------------------------------------

Valor por omisi¢n:  KEEPBINARY = off

Cuando PGP lee un fichero ".asc", reconoce que est† en formato radix-64,
y lo convierte en binario antes de procesarlo normalmente, dejando como
producto secundario un texto cifrado ".pgp" en forma binaria. DespuÇs de
seguir procesando el fichero ".pgp" para desencriptarlo, el fichero
final de salida se encontrar† en texto normal.

Puede que quiera borrar el fichero intermedio ".pgp", o dejar que PGP lo
haga autom†ticamente. Siempre puede volver a ejecutar PGP sobre el
fichero original ".asc".

El par†metro KEEPBINARY activa o desactiva la conservaci¢n del fichero
intermedio ".pgp" durante la desencriptaci¢n.

Para obtener m†s informaci¢n, consulte la secci¢n "Env°o de texto
cifrado por canales de correo-E: formato Radix-64" en el volumen sobre
Temas esenciales.


COMPRESS - Activar compresi¢n
=----------------------------

Valor por omisi¢n:  COMPRESS = on

El par†metro COMPRESS activa o desactiva la compresi¢n de datos para
antes de encriptar. Se utiliza sobre todo en la depuraci¢n de PGP.
Normalmente, PGP intenta comprimir el texto normal antes de encriptarlo.
En general, debe dejarse esta opci¢n tal como est†, y permitir a PGP que
intente comprimir el texto.


COMPLETES_NEEDED - Referencias completamente fiables necesarias
=--------------------------------------------------------------

Valor por omisi¢n:  COMPLETES_NEEDED = 1

El par†metro COMPLETES_NEEDED indica el m°nimo n£mero de referencias
completamente fiables que se necesitan para certificar por completo una
clave p£blica en el fichero. Se proporciona as° una forma de ajustar el
escepticismo de PGP.

Para obtener m†s informaci¢n, consulte la secci¢n "®C¢mo controla PGP la
validez de las claves?" en el volumen sobre Temas esenciales de la Gu°a
del usuario.


MARGINALS_NEEDED - Referencias relativamente fiables necesarias
=--------------------------------------------------------------

Valor por omisi¢n:  MARGINALS_NEEDED = 2

El par†metro MARGINALS_NEEDED indica el m°nimo n£mero de referencias
relativamente fiables que se requieren para certificar completamente una
clave p£blica en el fichero. Se proporciona as° una forma de ajustar el
escepticismo de PGP.

Para obtener m†s informaci¢n, consulte la secci¢n "®C¢mo controla PGP la
validez de las claves?" en el volumen sobre Temas esenciales de la Gu°a
del usuario.


CERT_DEPTH - Profundidad de anidamiento de referencias
=-----------------------------------------------------

Valor por omisi¢n:  CERT_DEPTH = 4

El par†metro CERT_DEPTH indica a quÇ nivel de profundidad pueden
anidarse las referencias para certificar otras que certifiquen claves
p£blicas en el fichero. Por ejemplo, si CERT_DEPTH es 1, s¢lo puede
haber una capa de referencias debajo de la clave fundamentalmente
fiable. Si este fuese el caso, usted tendr°a que certificar directamente
las claves p£blicas de todas las referencias fiables del fichero. Si
hace CERT_DEPTH igual a 0, no podr† hacer uso de referencias, y tendr†
que certificar directamente todas y cada una de las claves en el fichero
antes de poder utilizarlas. El m°nimo valor para CERT_DEPTH es 0, y el
m†ximo 8.

Para obtener m†s informaci¢n, consulte la secci¢n "®C¢mo controla PGP la
validez de las claves?" en el volumen sobre Temas esenciales de la Gu°a
del Usuario.


BAKRING - Nombre de la copia de seguridad del fichero secreto
=------------------------------------------------------------

Valor por omisi¢n:  BAKRING = ""

Todas las certificaciones que PGP hace en el fichero de claves p£blicas
dependen en £ltima instancia de su propia clave (o claves)
fundamentalmente fiable. Para detectar cualquier manipulaci¢n del
fichero, PGP debe comprobar que su propia clave no ha sido manipulada.
Para hacerlo, PGP debe comparar la clave p£blica con una copia de
seguridad en un medio resistente a manipulaci¢n, tal como un disco
protegido contra escritura. La clave secreta contiene la informaci¢n de
la clave p£blica, m†s algunos elementos secretos. Por tanto, PGP puede
comprobar la clave p£blica con una copia de seguridad de la clave
secreta.

El par†metro BAKRING indica quÇ trayectoria se utiliza para encontrar la
copia fiable del fichero de claves secretas. En DOS, podr°a establecerse
como "a:\secring.pgp", para indicar una copia de seguridad protegida
contra escritura en la unidad de diskette. La operaci¢n s¢lo se realiza
con la opci¢n -kc de PGP para comprobar el fichero de claves p£blicas.

Si no se define BAKRING, PGP no comprueba la clave con una copia de
seguridad.

Para obtener m†s informaci¢n, consulte las secciones "C¢mo proteger las
claves p£blicas contra manipulaci¢n" y "®C¢mo controla PGP la validez de
las claves?" en el volumen de Temas esenciales.


PAGER - Elegir la orden de coraza para mostrar texto normal
=----------------------------------------------------------

Valor por omisi¢n:  PAGER = ""

PGP le permite visualizar en la pantalla el texto normal descifrado
(como con la orden "more" al estilo Unix), sin escribirlo en un fichero,
indicando la opci¢n -m (more) {m†s} al desencriptar. Se muestra el texto
normal pantalla por pantalla.

Si prefiere utilizar una utilidad de paginaci¢n m†s ex¢tica en lugar de
la integrada en PGP, puede indicar el nombre de una orden de coraza que
pueda invocarse para mostrar el fichero de texto normal. El par†metro
PAGER indica la orden para mostrar el fichero. Por ejemplo, en sistemas
DOS, puede que quiera utilizar el conocido programa compartido
"list.com" para mostrar el texto. Asumiendo que tenga una copia de
"list.com", puede establecer PAGER:

PAGER = "list"

No obstante, si el remitente hubiese indicado que el fichero es s¢lo
para sus ojos, y que no puede escribirse en el disco, PGP utilizar† su
propia funci¢n interna de visualizaci¢n.

Para obtener m†s informaci¢n, consulte la secci¢n "Visualizaci¢n del
texto descifrado en la pantalla".


SHOWPASS - Mostrar al usuario la frase de contrase§a
=---------------------------------------------------

Valor por omisi¢n:  SHOWPASS = off

En general, PGP no permite que se vea la frase de contrase§a mientras se
escribe. As° se dificulta que alguien pueda mirar por encima del hombro
mientras lo hace, consiguiendo la contrase§a. Sin embargo, algunas
personas que no pueden o no saben escribir a m†quina tienen problemas
para introducir la frase de contrase§a sin mirar lo que est†n
escribiendo, y despuÇs de todo quiz† lo estÇn haciendo en la intimidad
de sus propias casas. Ellos pidieron que PGP pudiese configurarse para
poder ver lo que estaban escribiendo al introducir la frase.

El par†metro SHOWPASS permite que PGP muestre lo que se escribe al
introducir la frase de contrase§a.


TZFIX - Ajuste de zona horaria
=-----------------------------

Valor por omisi¢n:  TZFIX = 0

PGP proporciona sellos de hora para las claves y los certificados de
firma en Greenwich Mean Time (GMT) {Hora media de Greenwich}, o
Coordinated Universal Time (UTC) {Hora coordinada universal}, que para
este caso son equivalentes. Cuando PGP pide al sistema la hora, se
supone que Çste la da en GMT.

Sin embargo, algunas veces, debido a una configuraci¢n incorrecta de los
sistemas MSDOS, la hora del sistema se devuelve en US Pacific Standard
Time {Hora est†ndar del Pac°fico en US} m†s 8 horas. Extra§o, ®no?
Quiz† debido a alg£n tipo de patrioter°a de la costa oeste de los EEUU,
MSDOS supone que la hora local es US Pacific time, y la corrige hacia
GMT. Este comportamiento afecta negativamente a la funci¢n interna GMT
de MSDOS a la que llama PGP. En cualquier caso, si la variable TZ de
entorno MSDOS est† bien definida para su zona horaria, se corrige la
idea equivocada que tiene MSDOS de que todo el mundo vive en la costa
oeste de los EEUU.

El par†metro TZFIX indica el n£mero de horas que hay que a§adir a la
funci¢n de hora del sistema para obtener GMT y poner los sellos de hora
GMT en las claves y en las firmas. Si la variable TZ del entorno MSDOS
est† definida adecuadamente, puede dejarse TZFIX=0. En general, los
sistemas Unix no tendr†n que preocuparse de establecer TZFIX. Sin
embargo, si est† utilizando alg£n oscuro sistema operativo que no sabe
nada de GMT, tendr† que utilizar TZFIX para ajustar la hora.

En sistemas MSDOS sin TZ definida, puede hacerse TZFIX=0 para
California, -1 para Colorado, -2 para Chicago, -3 para Nueva York, -8
para Londres, -9 para Amsterdam {y Madrid}. En verano, habr† que
disminuir esos valores a mano. Un engorro.

Es mucho m†s claro establecer la variable TZ de entorno MSDOS dentro
del fichero AUTOEXEC.BAT, y no utilizar la correcci¢n TZFIX. De esa
manera, MSDOS proporciona sellos apropiados de hora GMT, y corrige
autom†ticamente los ajustes horarios para ahorro de energ°a (en EEUU)
{horario de invierno}. He aqu° algunas l°neas de ejemplo para
AUTOEXEC.BAT, dependiendo de la zona horaria:

Para Los Angeles: SET TZ=PST8PDT
Para Denver:      SET TZ=MST7MDT
Para Arizona:     SET TZ=MST7
     (Arizona nunca tiene horario de ahorro de energ°a)
Para Chicago:     SET TZ=CST6CDT
Para Nueva York:  SET TZ=EST5EDT
Para Londres:     SET TZ=GMT0BST
Para Amsterdam:   SET TZ=MET-1DST
     (y resto de la Comunidad Europea)
Para Mosc£:       SET TZ=MSK-3MSD
Para Aukland:     SET TZ=NZT-13


CLEARSIG - Activar Mensajes firmados encapsulados como texto normal
=------------------------------------------------------------------

Valor por omisi¢n:  CLEARSIG = off

En general, los mensajes de PGP firmados y sin encriptar tienen un
certificado de firma delante en forma binaria. Para enviarlo por medio
de un canal de correo-E de 7 bits, se aplica la armadura ASCII radix-64
(consulte el par†metro ARMOR), haciendo que el mensaje sea ilegible para
los ojos humanos, aunque el mensaje no est† cifrado. El destinatario
debe utilizar PGP para eliminar la armadura antes de leer el mensaje.

Si el mensaje original es texto (no binario), hay una forma de enviarlo
por medio de un canal de correo-E de forma que se aplique la armadura
ASCII solamente al certificado binario de firma, pero no al texto
normal. De esta manera se puede leer el mensaje sin la ayuda de PGP. Por
supuesto, har† falta PGP para comprobar la firma.

Para activar esta caracter°stica, establezca CLEARSIG=ON y ARMOR=ON (o
utilice la opci¢n -a), y establezca TEXTMODE=ON (o utilice la opci¢n -
t). Por ejemplo, podr°a establecer CLEARSIG directamente desde la l°nea
de ¢rdenes:

pgp -sta +clearsig=on mensaje.txt

Esta representaci¢n de los mensajes es an†loga al tipo MIC-CLEAR de
mensaje del Internet Privacy Enhanced Mail (PEM). TambiÇn es importante
resaltar que, dado que este mÇtodo s¢lo aplica la armadura ASCII al
certificado binario de firma, y no al texto mismo del mensaje, hay alg£n
riesgo de que el mensaje sin armadura pueda sufrir alguna interferencia
durante el camino. Puede ocurrir si pasa por alguna pasarela de correo-E
que realice alguna conversi¢n del conjunto de caracteres, o, en ciertos
casos, pueden a§adirse o eliminarse espacios del final de las l°neas. Si
ocurre esto, la firma no se podr† verificar, dando una indicaci¢n
incorrecta de alteraci¢n intencionada. Sin embargo, dado que PEM tiene
una debilidad similar, parece £til tener esta caracter°stica a pesar de
sus riesgos.

Desde la versi¢n 2.2 de PGP, se ignoran los espacios al final de la
l°nea para calcular la firma en modo CLEARSIG.


VERBOSE - Activar modo verboso
=-----------------------------

Valor por omisi¢n :  VERBOSE = 1

VERBOSE puede establecerse como 0, 1 o 2, dependiendo de lo detallados
que quiera ver los mensajes diagn¢sticos de PGP. Los valores son:

0 - S¢lo mostrar mensajes si hay problemas. Los aficionados a Unix
quer°an este "modo silencioso".

1 - Normal. Muestra una cantidad razonable de informaci¢n en los
mensajes de diagn¢stico y recomendaci¢n.

2 - Muestra la m†xima informaci¢n, normalmente para ayudar a
diagnosticar problemas en PGP. No se recomienda para uso normal. Adem†s,
PGP no tiene problemas, ®eh?


INTERACTIVE - Pedir confirmaci¢n para a§adir claves
=--------------------------------------------------

Valor por omisi¢n:  INTERACTIVE = off

La activaci¢n de este modo hace que, al a§adir un fichero de con varias
claves al fichero, PGP pida confirmaci¢n para cada clave antes de
a§adirla.


Protecci¢n contra falsos sellos de hora
=======================================

Un punto vulnerable poco evidente en PGP implica a aquellos usuarios
deshonestos que ponen falsos sellos de hora en sus certificados de clave
p£blica y en sus firmas. Puede saltarse esta secci¢n si es usted un
usuario ocasional, sin interÇs especial en los oscuros protocolos de
clave p£blica.

No hay nada que pueda evitar que un usuario deshonesto cambie la fecha y
la hora del reloj de su propio sistema, y genere certificados de clave
p£blica y de firma para que parezcan haber sido elaboradas en otro
momento. Puede conseguir que parezca que ha firmado algo antes o despuÇs
del momento real, o que su clave p£blica o secreta ha sido creada antes
o despuÇs. Este comportamiento puede producirle alg£n beneficio legal o
monetario, por ejemplo creando una v°a de escape que le permita negar
una firma.

Una soluci¢n para este caso podr°a consistir en involucrar a una
autoridad de certificaci¢n o notario, que crear°a firmas certificadas
con un sello de hora fiable. No tiene por quÇ ser una autoridad
centralizada. Cualquier referencia de confianza o tercera parte podr°a
realizar esta funci¢n, del mismo modo que lo hacen los verdaderos
notarios. Un certificado de clave p£blica podr°a estar firmado por el
notario, y ese sello de hora fiable en su firma podr°a tener alguna
relevancia legal. El notario podr°a introducir el certificado firmado en
un registro especial controlado por Çl mismo. Cualquiera podr°a
consultar ese registro.

El notario tambiÇn podr°a firmar otras firmas, creando un certificado de
firma de un certificado de firma. As° servir°a como testigo de la firma,
del mismo modo que hacen los verdaderos notarios en papel. De nuevo, el
notario podr°a incluir el certificado de firma separado (sin todo el
documento) en un registro controlado por Çl mismo. La firma del notario
tendr°a un sello fiable de hora, que podr°a tener mayor credibilidad que
el de la firma original. Una firma se har°a "legal" si ese notario la
firma y la registra.

El problema de la certificaci¢n de firmas con notarios y sellos fiables
de hora merece un tratamiento m†s detallado. No voy a intentar ahora
desenredar esta madeja¯. Se puede encontrar una buena exposici¢n del
tema en el art°culo de Denning en IEEE Computer (1983, vÇanse las
referencias). Quedan muchos detalles por resolver en estos esquemas de
certificaci¢n. Seguir†n complet†ndose al aumentar la utilizaci¢n de PGP,
y cuando otros productos de clave p£blica desarrollen sus propios
esquemas de certificaci¢n.



Una mirada en el interior
=========================

Echemos un vistazo a unas cuantas caracter°sticas internas de PGP.


N£meros aleatorios
=-----------------

PGP utiliza un generador criptogr†ficamente fuerte de n£meros
pseudoaleatorios, para generar las claves temporales de las sesiones
convencionales. El fichero de semilla que necesita se llama
"randseed.bin". Puede estar en cualquier directorio, indicado por la
variable PGPPATH. Si no existe ese fichero, se crea autom†ticamente y se
rellena de n£meros verdaderamente aleatorios obtenidos midiendo tiempos
entre pulsaciones de tecla.

El generador vuelve a construir el fichero cada vez que lo utiliza,
mezclando nuevos datos de claves derivados parcialmente con la hora y
otras fuentes verdaderamente aleatorias. Se utiliza el algoritmo de
cifrado convencional como motor del generador de n£meros aleatorios. Ese
fichero contiene material aleatorio para semillas y datos aleatorios de
claves, para poner a punto el motor de cifrado convencional como
generador de n£meros aleatorios.

Este fichero de semillas deber°a estar al menos ligeramente protegido
contra exposici¢n, para reducir el riesgo de que un atacante pudiese
derivar las clave siguiente o anterior. El atacante no lo pasar°a muy
bien para sacar algo £til de este fichero, ya que se lava
criptogr†ficamente antes y despuÇs de cada utilizaci¢n. Sin embargo,
parece prudente al menos intentar que no caiga en las manos equivocadas.

Si no se encuentra c¢modo confiando en una fuente de n£meros aleatorios
derivada algor°tmicamente, por resistente que sea, tenga en cuenta que
ya se f°a del mismo cifrado convencional para proteger los mensajes. Si
es lo suficientemente resistente para eso, deber°a bastar como fuente de
n£meros aleatorios para claves temporales. Tenga en cuenta que PGP sigue
utilizando n£meros verdaderamente aleatorios, obtenidos de fuentes
f°sicas (sobre todo mediciones de tiempos en el teclado), para generar
los pares de claves p£blica/secreta que se utilizan a largo plazo.


El algoritmo de cifrado convencional de PGP
=------------------------------------------

Como se ha descrito anteriormente, PGP "va a caballo"¯ de un algoritmo
de cifrado convencional de clave £nica, utilizando un algoritmo de clave
p£blica para cifrar la clave de la sesi¢n convencional y conmutando
entonces a ese tipo de criptograf°a convencional m†s r†pida. Hablemos
ahora de este algoritmo de cifrado convencional. No es DES.

El Federal Data Encryption Standard {Norma Federal de Cifrado de Datos}
(DES) resulta un buen algoritmo para la mayor°a de las aplicaciones
comerciales. Sin embargo, el Gobierno no conf°a en DES para proteger sus
propios datos clasificados. Quiz† se deba a que la longitud de la clave
DES es 56 bits, lo bastante corta como para permitir un ataque por
fuerza bruta mediante una m†quina especial, construida a base de grandes
cantidades de chips DES. Por otra parte, Biham y Shamir han tenido
recientemente un cierto Çxito atacando el DES completo de 16 rondas.

PGP no utiliza DES como algoritmo convencional para encriptar mensajes.
En su lugar, PGP emplea un algoritmo de cifrado convencional de clave
£nica por bloques, llamado IDEA(mr). Una versi¢n posterior de PGP podr†
tambiÇn admitir DES como opci¢n, siempre que lo solicite un n£mero
suficiente de usuarios. Sin embargo, sospecho que IDEA es mejor que DES.

Para los que tengan interÇs en criptograf°a, el cifrado IDEA tiene un
tama§o de bloque de 64 bits para el texto normal y el texto cifrado.
Tiene un tama§o de clave de 128 bits. Su dise§o se basa en el concepto
de "mezclar operaciones de distintos grupos algebraicos". En aplicaci¢n
inform†tica, es mucho m†s r†pido que DES. Del mismo modo que este, puede
utilizarse en modo de retroalimentaci¢n de cifrado (CFB) y
encadenamiento de bloques de cifrado (CBC). PGP utiliza el modo CFB de
64 bits.

El cifrado por bloques IPES/IDEA fue desarrollado en ETH, Zurich, por
James L. Massey y Xuejia Lay, y publicado en 1990. No es un algoritmo
"hecho en casa". Sus dise§adores tienen una reputaci¢n distinguida en la
comunidad criptol¢gica. Los primeros art°culos sobre el algoritmo lo
llamaban IPES (Improved Proposed Encryption Standard) {Norma mejorada
propuesta para cifrado}, pero despuÇs se cambi¢ el nombre a IDEA
(International Data Encryption Algorithm) {Algoritmo Internacional de
Cifrado de Datos}. Hasta el momento, IDEA ha resistido los ataques mucho
mejor que otros cifrados como FEAL, REDOC-II, LOKI, Snefru y Khafre.
Adem†s, los primeros datos sugieren que IDEA es m†s resistente que DES
al fruct°fero ataque criptoanal°tico diferencial de Biham & Shamir.
Biham y Shamir han examinado el cifrado IDEA para encontrar sus
debilidades, sin Çxito. Varios grupos criptoanal°ticos acadÇmicos en
BÇlgica, Inglaterra y Alemania tambiÇn est†n intentando atacarlo, as°
como grupos militares de diversos pa°ses europeos. Seg£n este nuevo
cifrado sigue atrayendo esfuerzos de los centros m†s formidables del
mundo criptoanal°tico, la confianza en IDEA crece con el tiempo.

Un famoso jugador de hockey dijo una vez una frase que se traduce como
"Intento patinar hacia donde creo que va a estar la pastilla". Mucha
gente empieza a creer que los d°as de DES est†n contados. Yo voy
patinando hacia IDEA.

No es pr†ctico, ergon¢micamente, utilizar RSA puro con grandes claves
para encriptar y desencriptar mensajes largos. Absolutamente nadie lo
hace as° en el mundo real. Sin embargo, puede que a usted le preocupe
que el conjunto se vea debilitado por utilizar un esquema h°brido de
clave p£blica y convencional, s¢lo para acelerar las cosas. DespuÇs de
todo, una cadena es tan fuerte como el m†s dÇbil de sus eslabones.
Muchas personas, con menos experiencia en criptograf°a, creen
equivocadamente que RSA es intr°nsecamente m†s resistente que cualquier
cifrado convencional. No es as°. RSA puede hacerse dÇbil si se utilizan
claves dÇbiles, y los cifrados convencionales pueden hacerse resistentes
eligiendo buenos algoritmos. Es dif°cil saber exactamente la resistencia
de un buen cifrado convencional sin llegar a romperlo. Un cifrado
convencional verdaderamente bueno puede ser incluso m†s dif°cil de
romper que una clave RSA de "grado militar". El aliciente de la
criptograf°a de clave p£blica no viene de ser intr°nsecamente m†s
resistente que el cifrado convencional-- su atractivo est† en que hace
mucho m†s c¢moda la gesti¢n de las claves.


Compresi¢n de datos
=------------------

Normalmente, PGP comprime el texto normal antes de encriptarlo. DespuÇs
es demasiado tarde para comprimir; los datos cifrados son
incomprimibles. La compresi¢n ahorra tiempo de transmisi¢n por modem y
espacio en el disco, y, lo que es m†s importante, refuerza la seguridad
criptogr†fica. La mayor°a de las tÇcnicas criptoanal°ticas explotan las
redundancias del texto normal para romper el cifrado. La compresi¢n de
datos reduce la redundancia en el texto, mejorando la resistencia al
criptoan†lisis. Lleva m†s tiempo comprimir el texto normal, pero merece
la pena desde el punto de vista de la seguridad, al menos en mi cauta
opini¢n.

PGP no comprime los ficheros demasiado cortos, ni los que no se pueden
comprimir bien.

Si lo prefiere, puede utilizar PKZIP para comprimir el texto normal
antes de encriptarlo. PKZIP es una utilidad de compresi¢n para DOS, en
programaci¢n compartida, efectiva y f†cil de encontrar, de PKWare Inc.
TambiÇn puede utilizar ZIP, un programa gratuito de compresi¢n,
compatible con PKZIP, para Unix, DOS y otros sistemas, que puede pedir a
Jean-loup Gailly. En algunos casos, resulta una ventaja utilizar PKZIP o
ZIP, porque, al contrario que el algoritmo de compresi¢n incorporado en
PGP, PKZIP y ZIP pueden comprimir m£ltiples ficheros en uno solo, que
vuelve a dividirse en varios al descomprimir. PGP no intenta volver a
comprimir un fichero que ya ha comprimido. DespuÇs de desencriptar, el
destinatario puede descomprimir el texto normal con PKUNZIP. Si el texto
descifrado es un fichero comprimido PKZIP, PGP lo reconoce
autom†ticamente y se lo indica al destinatario.

Para aquellos lectores con interÇs tÇcnico, la versi¢n actual de PGP
utiliza las rutinas gratuitas de compresi¢n ZIP escritas por Jean-loup
Gailly, Mark Adler y Richard B. Wales. Este programa ZIP utiliza
algoritmos de compresi¢n funcionalmente equivalentes a los que utiliza
el nuevo PKZIP 2.0. Se eligi¢ para PGP principalmente por la
disponibilidad del c¢digo fuente en C, gratuito y transportable, porque
tiene una muy buena proporci¢n de compresi¢n y porque es r†pido.

Peter Gutmann ha escrito tambiÇn una interesante utilidad de compresi¢n
llamada HPACK, disponible gratuitamente en muchos establecimientos FTP
de Internet. Encripta los archivos comprimidos, utilizando formatos de
datos y ficheros de claves de PGP. El me pidi¢ que lo mencionase aqu°.


Res£menes de mensajes y firmas digitales
=---------------------------------------

Para elaborar una firma digital, PGP encripta con la clave secreta. Sin
embargo, PGP no encripta el mensaje completo con esa clave-- llevar°a
demasiado tiempo. En lugar de eso, PGP encripta un "resumen de mensaje".

El resumen de mensaje es una "destilaci¢n" compacta (128 bits) del
mensaje, basada en la misma idea general de una suma de verificaci¢n.
Puede tambiÇn considerarse una "huella dactilar" del mensaje. El resumen
"representa" el mensaje, de forma que, si este se alterase de alguna
manera, se calcular°a un resumen distinto. As° se hace posible detectar
cualquier cambio que haya podido introducir un falsificador. El resumen
se calcula mediante una funci¢n "distribuci¢n" ("hash") unidireccional y
criptogr†ficamente resistente del mensaje. Resultar°a computacionalmente
imposible para un atacante idear un mensaje sustitutivo que produjese un
resumen idÇntico. En este aspecto, un resumen de mensaje es mucho mejor
que una suma de verificaci¢n, porque ser°a f†cil escribir un mensaje que
produjese la misma suma. Y, al igual que ocurre con la suma de
verificaci¢n, no se puede derivar el mensaje original partiendo del
resumen.

El resumen del mensaje no basta por s° mismo para autenticar un mensaje.
El algoritmo del resumen es p£blico, y no se necesita saber ninguna
clave secreta para calcularlo. Si s¢lo a§adiÇsemos el resumen al
mensaje, el falsificador podr°a alterarlo y simplemente a§adir el nuevo
resumen, calculado a partir del mensaje cambiado. Para proporcionar
verdadera autenticaci¢n, el remitente tiene que encriptar (firmar) el
resumen del mensaje con su propia clave secreta.

El remitente calcula el resumen del mensaje. Su clave secreta se utiliza
para encriptar ese resumen junto a un sello electr¢nico de hora,
formando as° una firma digital o certificado de firma. El remitente
env°a la firma digital junto con el mensaje. El destinatario recibe el
mensaje y la firma digital, y obtiene el resumen original desde la
firma, comprob†ndola con la clave p£blica del remitente. Calcula
entonces un nuevo resumen del mensaje, y comprueba que coincide con el
que se ha obtenido de la firma digital. Si es as°, se prueba que el
mensaje no ha sido alterado, y que realmente proviene del destinatario
al que pertenece la clave p£blica con la que se ha comprobado la firma.

Un posible falsificador tendr°a que producir un mensaje modificado que
produjese un resumen idÇntico (lo que resulta imposible), o tendr°a que
crear una nueva firma digital de otro resumen (tambiÇn imposible sin la
clave secreta del remitente).

Las firmas digitales prueban quiÇn envi¢ el mensaje, y que Çste no ha
sido alterado, bien por error o intencionadamente. TambiÇn proporciona
no-desentendimiento, que significa que el remitente no puede
desautorizar f†cilmente su firma.

La utilizaci¢n de res£menes de mensaje en las firmas digitales tiene
otras ventajas, aparte de ser m†s r†pida que firmar el mensaje completo
con la clave secreta. Los res£menes permiten que las firmas sean de un
tama§o peque§o, fijo y normalizado, sin importar el tama§o del mensaje
real. TambiÇn permite que el programa compruebe la integridad del
mensaje autom†ticamente, de forma parecida a como se realiza una suma de
verificaci¢n. Adem†s, permite que las firmas se almacenen separadas de
los mensajes, quiz† incluso en un archivo p£blico, sin revelar
informaci¢n confidencial sobre ellos, ya que nadie puede derivar el
contenido de un mensaje partiendo de su resumen.

El algoritmo que se utiliza es el Algoritmo de Resumen de Mensaje MD5,
donado al dominio p£blico por RSA Data Security, Inc. El dise§ador de
MD5, Ronald Rivest, escribe sobre MD5:

[traducci¢n]
"Se conjetura que la dificultad de obtener dos mensajes con el mismo
resumen es del orden de 2^64 operaciones, y que la dificultad de obtener
un mensaje que tenga un resumen determinado es del orden de 2^128
operaciones. El algoritmo MD5 ha sido comprobado cuidadosamente para
encontrar debilidades. En cualquier caso, es relativamente nuevo, y un
mayor an†lisis de su seguridad est† justificado, naturalmente, como
ocurre con cualquier propuesta de este tipo. El nivel de seguridad que
proporciona MD5 deber°a bastar para implementar esquemas h°bridos de
alta seguridad en firma digital, basados en MD5 y el criptosistema RSA
de clave p£blica."


Compatibilidad con las versiones anteriores de PGP
==================================================

Esta versi¢n de PGP no es compatible con la versi¢n 1.0. Si tiene claves
para PGP 1.0, tendr† que generar unas nuevas. Esta versi¢n utiliza
nuevos algoritmos de criptograf°a convencional, compresi¢n y res£menes
de mensaje, adem†s de aplicar un enfoque mucho mejor a la gesti¢n de
claves. Eran demasiados cambios para hacerla compatible con los antiguos
mensajes, firmas y claves. Podr°amos haber ofrecido una utilidad de
conversi¢n para transformar las claves antiguas en claves nuevas, pero
est†bamos todos cansados y quer°amos sacar la nueva versi¢n. Por otra
parte, la conversi¢n de antiguas claves en nuevas podr°a haber creado
m†s problemas de los que hubiese solucionado, ya que hemos cambiado a un
nuevo estilo recomendado, m†s uniforme, para el identificador de
usuario, que incluye el nombre completo y la direcci¢n de correo-E con
una sintaxis determinada.

Hay compatibilidad desde la versi¢n 2.0 a las superiores. Dado que se
a§aden nuevas caracter°sticas, las versiones antiguas no siempre ser†n
capaces de tratar algunos ficheros creados con las nuevas.

Hemos hecho un esfuerzo para dise§ar las estructuras internas de datos
de esta versi¢n de PGP para que puedan adaptarse a futuros cambios, por
lo que esperamos que no haga falta desechar y regenerar las claves en
las pr¢ximas versiones.



Puntos vulnerables
==================

Ning£n sistema de seguridad de datos es impenetrable. PGP puede burlarse
de diversas maneras. En cualquier sistema de seguridad, debe preguntarse
usted mismo si la informaci¢n que intenta proteger es m†s valiosa para
el atacante que el coste del ataque. As°, usted puede protegerse de los
ataques m†s baratos, sin preocuparse de los m†s caros.

Parte de la exposici¢n que sigue puede parecer demasiado paranoica, pero
es la actitud apropiada para debatir razonablemente sobre
vulnerabilidad.


Compromiso de la frase de contrase§a y de la clave secreta
=---------------------------------------------------------

Seguramente, el ataque m†s simple est† relacionado con dejarse escrita
en alg£n sitio la frase de contrase§a. Si alguien la consiguiese, junto
al fichero de la clave secreta, podr°a leer los mensajes dirigidos a
usted y hacer firmas en su nombre.

No elija contrase§as obvias, que resulten f†ciles de adivinar, tales
como el nombre de sus hijos o c¢nyuge. Si eligiese como contrase§a una
sola palabra, podr°a adivinarse f†cilmente haciendo que un ordenador
probase todas las palabras en el diccionario hasta encontrarla. Esa es
la raz¢n por la que una frase es mejor que una simple contrase§a. Un
atacante m†s completo podr°a hacer que su ordenador explorase un libro
de citas famosas para encontrarla. No es dif°cil encontrar una frase de
contrase§a que sea f†cil de recordar pero dif°cil de adivinar, eligiendo
expresiones creativamente absurdas u oscuras citas literarias.

Para obtener m†s informaci¢n, consulte la secci¢n "C¢mo proteger las
claves secretas contra exposici¢n", en el volumen sobre Temas especiales
de la Gu°a del usuario de PGP.


Manipulaci¢n de claves p£blicas
=------------------------------

Un punto muy vulnerable es la manipulaci¢n de las claves p£blicas. Quiz†
sea el punto dÇbil de m†s crucial importancia en un criptosistema de
clave p£blica, en parte porque los principiantes no lo reconocen
inmediatamente. La importancia de esta debilidad, y las contramedidas
higiÇnicas adecuadas, se explican con detalle en la secci¢n "C¢mo
proteger las claves p£blicas contra manipulaci¢n", en el volumen sobre
Temas esenciales.

En resumen:  Cuando utilice alguna clave p£blica, aseg£rese de que no ha
sido manipulada. S¢lo debe confiar en una nueva clave p£blica si la ha
obtenido directamente del due§o, o si la ha firmado alguien en quien
usted conf°a. Aseg£rese de que nadie pueda manipular su anillo de claves
p£blicas. Mantenga control f°sico tanto sobre su anillo de claves
p£blicas como el de claves secretas, preferiblemente en su propio
ordenador personal, en lugar de hacerlo en un sistema remoto
multiusuario. Haga una copia de seguridad de los dos anillos.


Ficheros "no borrados del todo"
=------------------------------

Otro posible problema de seguridad tiene como origen la forma de borrar
ficheros de la mayor parte de los sistemas operativos. Cuando se
encripta un fichero y despuÇs se suprime el original, el sistema
operativo no borra f°sicamente los datos. Simplemente marca los bloques
del disco como borrados, permitiendo as° que se utilicen m†s adelante.
Es algo parecido a tirar documentos confidenciales en papel al
contenedor de reciclado, en lugar de pasarlos por la destructora de
documentos. Los bloques del disco todav°a tienen los datos
confidenciales que se quer°an borrar, y con el tiempo otros datos se
sobrescribir°an encima de ellos. Si un atacante leyese esos bloques poco
despuÇs de haber sido marcados, podr°a recuperar el texto normal.

De hecho, podr°a incluso hacerlo accidentalmente, si por alg£n motivo
algo va mal con el disco y algunos ficheros se borran o estropean. Se
puede ejecutar un programa de recuperaci¢n de discos para reponer los
ficheros da§ados, pero esto implica a menudo recuperar tambiÇn otros
ficheros antiguos. Los ficheros confidenciales, que se cre°an eliminados
para siempre, podr°an reaparecer y ser inspeccionados por quien estÇ
recuperando el disco da§ado. Incluso al crear el mensaje original con un
procesador o editor de textos, esos programas podr°an estar creando
m£ltiples copias temporales del fichero en el disco, para uso interno.
El procesador de textos borra estas copias temporales cuando ha acabado,
pero los fragmentos confidenciales todav°a est†n en alguna parte del
disco.

Perm°tame que le cuente una autÇntica historia de terror. Ten°a una
amiga, casada y con hijos peque§os, que tuvo una vez una corta y no muy
seria relaci¢n il°cita. Escribi¢ una carta a su amante con el procesador
de textos, y borr¢ la carta despuÇs de enviarla. M†s adelante, cuando la
relaci¢n ya hab°a terminado, el disco se estrope¢ y tuvo que recuperar
la informaci¢n porque conten°a otros documentos importantes. Le pidi¢ a
su marido que salvase el disco, lo que parec°a perfectamente seguro, ya
que ella sab°a que hab°a borrado la carta inculpatoria. Su marido
ejecut¢ un programa comercial de recuperaci¢n de discos. Recuper¢ bien
los ficheros, incluyendo la carta borrada. La ley¢, y esto inici¢ una
tr†gica cadena de acontecimientos.

La £nica forma de evitar que el texto normal vuelva a aparecer es hacer
que se sobrescriban de alguna manera los ficheros borrados. A menos que
estÇ seguro de que los bloques se volver†n a utilizar pronto, debe tomar
medidas activas para sobrescribir el fichero de texto normal, y todos
los fragmentos que haya dejado el procesador de textos. El texto
original puede sobrescribirse despuÇs de encriptar, con la opci¢n -w
(wipe {limpiar}) de PGP. Puede encargarse de los fragmentos de texto
normal que quedan en el disco por medio de alguna utilidad que
sobrescriba todos los bloques libres. Por ejemplo, las Utilidades Norton
lo hacen en MSDOS.

Incluso si sobrescribe el texto normal del disco, un atacante resuelto y
con recursos podr°a recuperar los datos. Quedan leves restos magnÇticos
de los datos originales despuÇs de sobrescribir. A veces puede
utilizarse maquinaria especial de recuperaci¢n de discos para obtener
los datos.


Virus y caballos de Troya
=------------------------

Otro tipo de ataque puede involucrar virus o gusanos inform†ticos
hostiles, dise§ados especialmente, que infecten PGP o el sistema
operativo. Este hipotÇtico virus podr°a dise§arse para capturar la frase
de contrase§a, la clave secreta o los mensajes descifrados, y despuÇs
escribirlos secretamente en un fichero o enviarlos al due§o del virus
por una red. TambiÇn podr°a alterar el comportamiento de PGP para que
las firmas no se comprobasen adecuadamente. Este tipo de ataque es m†s
barato que el ataque criptoanal°tico.

La defensa contra este problema se incluye dentro de la defensa contra
virus en general. Hay diversos programas comerciales anti-virus,
relativamente £tiles, y tambiÇn procedimientos higiÇnicos que pueden
reducir en gran manera la posibilidad de una infecci¢n. Una descripci¢n
detallada de las medidas contra virus y gusanos queda fuera del alcance
de este documento. PGP no tiene ninguna defensa contra virus, y asume
que su ordenador personal es un entorno de ejecuci¢n seguro. Si aparece
un virus o gusano con esas caracter°sticas, esperemos que corra
r†pidamente la voz para advertir a todo el mundo.

Otro ataque parecido podr°a darse si alguien hiciese una ingeniosa
imitaci¢n de PGP, que se comportarse como PGP en la mayor°a de los
aspectos, pero que no funcionase adecuadamente. Por ejemplo, podr°a
haber sido mutilado para que no comprobase las firmas adecuadamente,
haciendo que se aceptasen certificados falsos. Esta versi¢n "caballo de
Troya" de PGP no ser°a dif°cil de crear, ya que el c¢digo fuente de PGP
es f†cil de encontrar, por lo que cualquiera podr°a modificarlo y
producir una imitaci¢n de PGP con forma de zombie lobotomizado, que
parecer°a autÇntico pero que seguir°a las ¢rdenes de su diab¢lico amo.
Esta versi¢n caballo de Troya de PGP podr°a circular ampliamente,
alegando ser m°a. Bastante molesto.

Deber°a tomarse la molestia de obtener su copia de PGP de una fuente
fiable, si esa expresi¢n puede aplicarse realmente. Podr°a tambiÇn
conseguirlo de varias fuentes independientes, y verificar las copias con
una utilidad de comparaci¢n de ficheros.

Hay otras formas de comprobar si PGP ha sido manipulado, mediante firmas
digitales. Si alguien en quien usted conf°a firma la versi¢n ejecutable
de PGP, respondiendo del hecho de que no ha sido infectada o manipulada,
puede estar razonablemente seguro de tener una copia buena. Podr°a
utilizar una versi¢n anterior, de confianza, para comprobar la firma de
la versi¢n sospechosa de PGP. En cualquier caso, esto no ayudar°a si el
sistema operativo estuviese infectado, ni detectar°a si la copia
original de PGP.EXE hab°a sido alterada maliciosamente para comprometer
su capacidad de comprobar firmas. Esta comprobaci¢n tambiÇn asume que
usted tiene una copia buena y de confianza de la clave p£blica para
verificar la firma del ejecutable PGP.


Fallos en la seguridad f°sica
=----------------------------

Un fallo en la seguridad f°sica podr°a hacer que alguien consiguiese
f°sicamente los ficheros de texto normal o los mensajes impresos. Un
oponente resuelto podr°a conseguirlo por robo, b£squeda en la basura,
b£squeda y captura irrazonable, o soborno, chantaje o infiltraci¢n en el
personal. Algunos de estos ataques son especialmente factibles contra
organizaciones pol°ticas de base, que dependen en gran parte de personal
voluntario. Se ha comentado ampliamente en la prensa que el COINTELPRO
del FBI ha recurrido al robo, la infiltraci¢n y las escuchas ilegales
contra grupos anti-guerra y otros grupos a favor de los derechos
civiles. Mire lo que ocurri¢ en el Hotel Watergate.

No se duerma en un falso sentimiento de seguridad s¢lo por tener una
herramienta criptogr†fica. Las tÇcnicas criptogr†ficas protegen los
datos solamente mientras est†n cifrados-- las violaciones directas de la
seguridad f°sica pueden comprometer datos normales o informaci¢n escrita
o hablada.

Este tipo de ataque es m†s barato que el criptoan†lisis de PGP.


Ataques Tempest {tempestad}
=--------------

Otro tipo de ataque, empleado por oponentes con buen equipamiento,
recurre a la detecci¢n remota de las se§ales electromagnÇticas del
ordenador. Este ataque, caro y bastante laborioso, probablemente sigue
siendo m†s barato que el ataque criptoanal°tico directo. Una furgoneta
con los instrumentos apropiados puede aparcar cerca de la oficina, y
recoger a distancia todas las pulsaciones de teclas y los mensajes de la
pantalla. As° se ver°an comprometidas todas las contrase§as, todos los
mensajes, etc. Este ataque puede frustrarse protegiendo adecuadamente
todo el equipo inform†tico y los cables de la red, de manera que no
emitan estas se§ales. Esta tecnolog°a de protecci¢n se llama "Tempest",
y la utilizan diversas agencias del Gobierno y contratistas del
ejÇrcito. Hay proveedores de maquinaria que suministran protecci¢n
Tempest comercial, aunque puede estar sujeta a alg£n tipo de licencia
del Gobierno. ®Por quÇ cree que el Gobierno querr°a restringir la
disponibilidad de la protecci¢n Tempest?


Exposici¢n en sistemas multiusuario
=----------------------------------

PGP fue dise§ado originalmente para ordenadores DOS de un solo usuario,
bajo su control f°sico directo. Yo utilizo PGP en casa, en mi propio PC,
y, a menos que alguien entre en mi casa o controle mis emisiones
electromagnÇticas, es probable que no vean mis ficheros de texto normal
ni mis claves secretas.

Pero ahora PGP tambiÇn funciona en sistemas multiusuario como Unix y
VAX/VMS. En sistemas como estos, hay un riesgo mucho mayor de exposici¢n
para el texto normal o las contrase§as. El administrador del sistema en
Unix, o un intruso ingenioso podr°a leer los ficheros normales, o
utilizar un programa especial para ver las pulsaciones de tecla o leer
quÇ hay en la pantalla. En un sistema Unix, cualquier usuario puede leer
a distancia la informaci¢n sobre el entorno, simplemente con la orden
"ps". Hay problemas parecidos en m†quinas DOS conectadas en red local.
El riesgo que sufre la seguridad depende de cada situaci¢n particular.
Algunos sistemas multiusuario pueden resultar seguros por ser de
confianza todos los usuarios, por tener medidas de seguridad que
protegen de los ataques de intrusos, o s¢lo porque no haya un n£mero
suficiente de intrusos interesados. Algunos sistemas Unix son seguros
porque s¢lo tienen un usuario-- hay hasta ordenadores port†tiles
utilizando Unix. No ser°a razonable simplemente excluir PGP de todos los
sistemas Unix.

PGP no se ha dise§ado para proteger los datos mientras est†n en texto
normal, o si se encuentran en un sistema comprometido. Tampoco puede
evitar que un intruso tome complejas medidas para leer la clave secreta
mientras se utiliza. Tendr† que detectar estos riesgos en los sistemas
multiusuario, y ajustar sus expectativas y su comportamiento de acuerdo
con ellos. Puede que su situaci¢n sea tal que deber°a considerar
ejecutar PGP solamente en un sistema aislado, de un solo usuario, bajo
su directo control f°sico. Eso es lo que hago yo, y es lo que
recomiendo.


An†lisis de tr†fico
=------------------

Incluso aunque el atacante no pueda leer el contenido de los mensajes
cifrados, puede inferir alguna informaci¢n £til observando de d¢nde
vienen los mensajes y ad¢nde van, su tama§o y la hora del d°a a la que
se env°an. Es parecido a lo que hace un atacante cuando mira la cuenta
de llamadas de larga distancia para ver a quiÇn se ha llamado y durante
cu†nto tiempo, aunque desconozca el contenido. Esto se llama an†lisis de
tr†fico. PGP por s° mismo no protege contra ello. La soluci¢n de este
problema requerir°a protocolos especializados de comunicaci¢n, dise§ados
para reducir la exposici¢n al an†lisis en su entorno de comunicaci¢n,
quiz† con asesoramiento criptogr†fico.


Criptoan†lisis
=-------------

Un ataque criptoanal°tico caro y formidable podr°a organizarlo alguien
con vastos recursos inform†ticos, tal como una agencia de informaci¢n
del Gobierno. Podr°an romper la clave RSA con un nuevo hallazgo secreto
sobre factorizaci¢n. Quiz† sea as°, pero hay que destacar que en ciertos
casos el Gobierno de los EEUU conf°a lo bastante en el algoritmo RSA
como para proteger sus propias armas nucleares, seg£n Ron Rivest. Los
acadÇmicos civiles han estado atac†ndolo continuamente y sin Çxito desde
1978.

Quiz† el Gobierno tenga alg£n mÇtodo clasificado para romper el
algoritmo de encriptaci¢n convencional IDEA(mr) que utiliza PGP. Eso es
la peor pesadilla de cualquier cript¢grafo. No puede existir ninguna
garant°a absoluta de seguridad para las implementaciones criptogr†ficas
pr†cticas.

A£n as°, parece justificado un cierto optimismo. Los dise§adores del
algoritmo IDEA est†n entre los mejores cript¢grafos de Europa. Ha
aguantado un profundo an†lisis de seguridad, as° como revisi¢n de otros
colegas entre los mejores criptoanalistas del mundo no clasificado.
Parece tener ciertas ventajas de dise§o respecto a DES en cuanto a
resistencia al criptoan†lisis diferencial que se ha empleado para romper
DES.

Por otra parte, hasta si este algoritmo tiene algunas debilidades
sutiles a£n no descubiertas, PGP comprime el texto normal antes de
encriptar, lo que deber°a reducirlas bastante. El trabajo computacional
que hace falta para romperlo es seguramente mucho m†s caro que el valor
del mensaje.

Si su situaci¢n justifica una preocupaci¢n sobre ataques de este
calibre, quiz† debiera ponerse en contacto con un asesor en seguridad de
datos para realizar un enfoque personal, hecho a la medida de sus
necesidades. Boulder Software Engineering, cuya direcci¢n y telÇfono
aparecen al final de este documento, puede proporcionar ese tipo de
servicio.


En resumen, sin una buena protecci¢n criptogr†fica de las comunicaciones
de datos podr°a haber resultado pr†cticamente trivial y quiz† rutinario
para un oponente interceptar los mensajes, especialmente aquellos
enviados por modem o por un sistema de correo-E. Si utiliza PGP y toma
precauciones razonables, el atacante tendr† que emplear mucho m†s
esfuerzo y dinero para invadir su intimidad.

Si se protege contra los ataques m†s simples, y conf°a en que su
intimidad no va a ser invadida por un atacante resuelto y con grandes
recursos, probablemente estÇ seguro con PGP. PGP le da una "intimidad
bastante buena".


Consideraciones legales
=======================


Marcas registradas, copyright y garant°as
=----------------------------------------

"Pretty Good Privacy", "Phil's Pretty Good Software", y la etiqueta
"Pretty Good" para programas y maquinaria inform†tica son marcas
registradas de Philip Zimmermann y Phil's Pretty Good Software. PGP es
(c) Copyright Philip R. Zimmermann, 1990-1993. Philip Zimmermann tambiÇn
tiene el copyright del Manual del usuario de PGP, adem†s del de
cualquier traducci¢n del manual o del programa.

El autor no asume responsabilidad por los da§os que resulten del uso de
este programa, aunque hayan sido causados por defectos en Çl, y no hace
ninguna declaraci¢n sobre su comerciabilidad o su adaptaci¢n a alg£n
prop¢sito espec°fico. Se suministra "tal como es", sin garant°a alguna,
expl°cita o impl°cita.


Derechos de patente sobre los algoritmos
=---------------------------------------

El sistema RSA de clave p£blica fue desarrollado en el MIT, que tiene
una patente sobre Çl (patente U.S: #4,405,829, emitida el 20 de
Septiembre de 1983). Una compa§°a de California llamada Public Key
Partners (PKP) tiene la licencia comercial exclusiva para vender y dar
licencias del sistema RSA de clave p£blica.

PKP no ha dado ninguna licencia para utilizar su algoritmo RSA en PGP.
El autor de esta implementaci¢n del algoritmo RSA la suministra s¢lo
para uso educativo. La obtenci¢n de la licencia para este algoritmo es
responsabilidad de usted, el usuario, no del autor de la implementaci¢n
inform†tica. El autor no asume responsabilidad alguna por aquellas
transgresiones de la patente que puedan ocurrir por la utilizaci¢n sin
licencia del algoritmo RSA en el ordenador del usuario sin licencia del
propietario de la patente RSA.

Los usuarios de otros pa°ses deben tener en cuenta que la patente RSA no
tiene efecto fuera de los EEUU, y que no hay patente de RSA en ning£n
otro pa°s. Las agencias federales pueden utilizarlo porque el Gobierno
ya pag¢ el desarrollo de RSA con ayudas de la National Science
Foundation y la Armada estadounidense. Las empresas que ya tienen
licencia de PKP quiz† puedan utilizar PGP, dependiendo de sus tÇrminos.

Escrib° PGP desde cero, con mi propia implementaci¢n del algoritmo RSA.
Antes de publicar PGP, obtuve una opini¢n legal formal y por escrito de
un abogado de patentes, con gran experiencia en programas. Estoy
convencido de que publicar PGP como yo lo hice no infringe ninguna ley
de patentes.

No s¢lo adquiri¢ PKP los derechos exclusivos de patente del
criptosistema RSA, sino tambiÇn los de otras tres patentes sobre
esquemas rivales de clave p£blica inventados por otros en la Universidad
de Stanford. De esta manera, se ha dado a una compa§°a la cerradura
legal para casi todos los criptosistemas pr†cticos de clave p£blica.
Incluso parecen estar reclamando derechos de patente sobre el mismo
concepto de criptograf°a de clave p£blica, sin importar quÇ ingeniosos
nuevos algoritmos originales inventen otros. Encuentro preocupante ese
tipo de monopolio exhaustivo, porque creo que la criptograf°a de clave
p£blica est† destinada a convertirse en una tecnolog°a crucial para la
protecci¢n de nuestras libertades civiles y de nuestra intimidad dentro
una sociedad cada vez m†s conectada. Como poco, pone en riesgo esas
herramientas vitales proporcionando al Gobierno un £nico punto de
presi¢n e influencia.

Hay negociaciones en curso con RSA DAta Security Inc (RSADSI), una
empresa hermana de PKP, para legalizar PGP en los EEUU. Se conseguir°a
integrando RSAREF en PGP. RSAREF es un conjunto de subrutinas de RSADSI
que implementan el algoritmo RSA. Las subrutinas RSAREF tendr°an que
utilizarse en lugar de las originales de PGP. Hay algunos obst†culos
tÇcnicos para lograrlo, pero puede encontrarse una soluci¢n si las
negociaciones con RSADSI contin£an favorablemente. Si RSAREF se integra
en PGP, RSADSI dar† una licencia para su uso no comercial en los EEUU.
Las versiones PGP de fuera de los EEUU no utilizar†n RSAREF, sino la
biblioteca original de PGP, m†s r†pida, para efectuar los c†lculos RSA.
RSADSI puede requerir que se cambie el nombre PGP para que todo esto sea
posible. Sigan conectados.

Parece seguro que habr† nuevos lanzamientos de PGP, sin importar el
resultado de los problemas de licencia con Public Key Partners. Si PKP
no da licencia para PGP, los futuros lanzamientos de PGP pueden no venir
de m°. Hay innumerables aficionados a PGP fuera de los EEUU, y muchos de
ellos son ingenieros de programaci¢n que quieren mejorar PGP y
promocionarlo, aparte de lo que yo pueda hacer. El segundo lanzamiento
de PGP fue el esfuerzo conjunto de un equipo internacional de ingenieros
de programaci¢n, que ha implementado mejoras en PGP con consejos m°os
sobre el dise§o. El lanzamiento lo hacieron Brako Lankester en los
Pa°ses Bajos y Peter Gutmann en Nueva Zelanda, fuera del alcance de la
ley de patentes de los EEUU. S¢lo sale en Europa y Nueva Zelanda, pero
puede llegar espont†neamente a los EEUU sin mi ayuda ni la de nadie del
equipo de desarrollo de PGP.

El cifrado convencional por bloques IDEA(mr) que utiliza PGP tiene una
patente en Europa, en poder de ETH y una compa§°a suiza llamada Ascom-
Tech AG. El n£mero de patente es PCT/CH91/00117. Las patentes
internacionales est†n pendientes. IDEA(mr) es una marca registrada de
Ascom-Tech AG. No se necesita ning£n pago de licencia para el uso no
comercial de IDEA. Ascom Tech AG ha dado permiso a PGP para utilizar el
cifrado IDEA, y no pone ninguna restricci¢n sobre el uso de PGP para
cualquier prop¢sito, incluyendo el comercial. No puede extraerse el
cifrado IDEA de PGP y ponerlo en otro producto comercial sin una
licencia. Los usuarios comerciales de IDEA pueden pedir informaci¢n
sobre licencias a Dieter Profos, Ascom Tech AG, Solothurn Lab, Postfach
151, 4502 Solothurn, Suiza, Tel +41 65 242885, Fax +41 65 235761.

Las rutinas de compresi¢n ZIP en PGP vienen de c¢digo fuente libre y
gratuito ("freeware"), con permiso del autor. No conozco ninguna patente
sobre el algoritmo ZIP, pero puede comprobarlo usted mismo.


Licencias y distribuci¢n
=-----------------------

En los EEUU, PKP controla, por medio de la ley de patentes, las
licencias del algoritmo RSA. Sin embargo, yo no pongo ninguna objeci¢n a
que se utilice o distribuya libremente mi programa PGP, sin pagarme
ning£n tipo de licencia. Debe mantener los datos de copyright en PGP,
junto con esta documentaci¢n. Sin embargo, si vive usted en los EEUU,
esto puede no satisfacer las obligaciones legales que tiene hacia PKP
por utilizar el algoritmo RSA, tal como se ha mencionado anteriormente.

PGP no es un programa compartido ("shareware"), es un programa libre y
gratuito ("freeware"). Un programa libre prohibido. Publicado como un
servicio a la comunidad. Distribuir PGP gratis animar† a mucha m†s gente
a utilizarlo, algo que tendr†, esperemos, un mayor impacto social. Esto
podr°a llevar a un conocimiento y utilizaci¢n m†s amplios del
criptosistema RSA de clave p£blica.

El c¢digo fuente de PGP est† disponible gratuitamente bajo la "Copyleft"
General Public License {Licencia p£blica general "Copyleft"} de la Free
Software Foundation (FSF). Se incluye una copia de esa licencia con el
c¢digo fuente de PGP.

Aparte, y quiz† en contra de alguna de las estipulaciones de la General
Public License de FSF, son aplicables los siguientes tÇrminos:

1)  Los tratamientos de PGP en revistas o libros pueden incluir
    fragmentos del c¢digo fuente de PGP y su documentaci¢n, sin
    restricciones.

2)  Aunque la licencia de FSF permite productos derivados para uso
    general, proh°be productos derivados para uso comercial. A pesar de
    esto, puedo ofrecerle una licencia especial si quiere derivar un
    producto comercial de algunas partes de PGP. Puede haber o no una
    tasa, seg£n el tipo de acuerdo al que quiera llegar con PKP. La
    conservaci¢n de mi nota de copyright y mi atribuci¢n bastar† en
    algunos casos. Ll†meme y lo trataremos. Soy muy f†cil de contentar.
    Esas licencias no le liberan de los requisitos de la patente.

Distribuya PGP tan ampliamente como le sea posible. DÇselo a todos sus
amigos. Si tiene acceso a algunas BBS, env°e el lote completo ejecutable
de PGP a tantas como le sea posible. Puede tambiÇn distribuir el c¢digo
fuente de PGP, si lo tiene. El lote ejecutable de PGP versi¢n 2.3a para
MSDOS contiene el programa ejecutable, la documentaci¢n, anillos de
claves de ejemplo, incluyendo mi propia clave p£blica, y firmas para el
programa y el manual, todo en un fichero comprimido por PKZIP llamado
PGP23A.ZIP. El lote del c¢digo fuente de PGP para MSDOS contiene todos
los ficheros del c¢digo fuente en C dentro de un fichero comprimido por
PKZIP llamado PGP23SRC.ZIP.

Puede obtener copias y actualizaciones gratuitas de PGP en miles de BBSs
en todo el mundo, o en otras fuentes p£blicas, como establecimientos FTP
de Internet. No me pida una copia a m° directamente, porque preferir°a
evitar problemas legales con PKP por el momento. No obstante, puedo
indicarle d¢nde puede obtenerlo.

DespuÇs de todo este trabajo, debo admitir que no me importar°a recibir
correo de admiradores de PGP, para calibrar su popularidad. D°game quÇ
piensa de Çl, y cu†ntos amigos suyos lo utilizan. TambiÇn son bien
recibidos informes sobre errores, y sugerencias para mejorar PGP. Quiz†
un pr¢ximo lanzamiento de PGP refleje esas sugerencias.

Este proyecto no ha sido subvencionado, y casi me ha devorado vivo. Esto
quiere decir que no puede contar con que conteste al correo, a menos que
s¢lo necesite una respuesta corta por escrito, e incluya un sobre
franqueado con su direcci¢n. Sin embargo, s° contesto el correo-E. Por
favor, escriba en inglÇs, porque no estoy muy fuerte en otros idiomas.
Si llama y no estoy, es mejor volver a probar despuÇs. No devuelvo
normalmente llamadas a larga distancia, a menos que diga en el mensaje
que puedo llamar a cobro revertido. Si necesita una cantidad
considerable de mi tiempo, estoy disponible como asesor contratado, y
naturalmente devuelvo esas llamadas.

El correo m†s inc¢modo que recibo es de alguna persona bienintencionada
que me env°a unos d¢lares y me pide una copia de PGP. No puedo
envi†rsela por las amenazas legales de PKP (o a£n peor-- algunas veces
las peticiones vienen de otros pa°ses, y estar°a arriesg†ndome a
quebrantar las leyes de control de exportaci¢n de material
criptogr†fico). Hasta en el caso de que no hubiese problemas legales al
enviarles una copia de PGP, normalmente no mandan suficiente dinero para
compensar el tiempo (quiz† $50 {unas 7.000 Ptas.} lo ser°a, si lo
estuviese vendiendo). No tengo la organizaci¢n necesaria para un negocio
de venta por correo de bajo coste y bajo volumen. No puedo simplemente
ignorar la petici¢n y quedarme con el dinero, porque seguramente lo
consideran el pago por cumplir su encargo. Para devolver el dinero,
tengo que coger el coche y llegar a la oficina de correos para comprar
sellos, porque estas peticiones no suelen incluir un sobre franqueado
con direcci¢n. Tengo adem†s que escribir una contestaci¢n amable
diciendo que no puedo enviar el programa. Si pospongo la respuesta y
dejo la carta en la mesa, puede quedar enterrada en cuesti¢n de minutos,
y sin volver a ver la luz del d°a durante meses. Multiplique estas
peque§as molestias por el n£mero de peticiones que tengo, y ver† el
problema. ®No basta que el programa sea gratuito?  Ser°a mucho mejor que
la gente intentase obtener PGP de alguna otra fuente de las muchas que
hay. Si no tiene modem, pida a un amigo que lo consiga para usted. Si no
puede encontrarlo, no me importa responder una r†pida llamada de
telÇfono.

Si alguien quiere prestarse como voluntario para mejorar PGP, h†gamelo
saber. Seguro que podr°a utilizar algo m†s de trabajo. Algunas
caracter°sticas tuvieron que posponerse para poder sacarlo. Desde
entonces, algunos usuarios de PGP han donado su tiempo para transportar
PGP a Unix para estaciones SPARC de Sun, a Ultrix. a VAX/VMS, a OS/2, al
Amiga y al Atari ST. Quiz† usted podr°a ayudar a transportarlo a alg£n
nuevo entorno. Por favor, d°game si tiene intenci¢n de transportar PGP,
para evitar la duplicaci¢n de esfuerzo, y para evitar que empiece con
una versi¢n obsoleta del c¢digo fuente.

Hay tantas traducciones de PGP que la mayor°a no se distribuye con la
versi¢n normal de PGP, porque necesitar°a demasiado espacio. Hay "lotes"
de traducci¢n disponibles en unas cuantas fuentes independientes, y a
veces de forma separada en los mismos centros de distribuci¢n que llevan
el programa PGP. Estos lotes incluyen las versiones traducidas de
LANGUAGE.TXT, PGP.HLP y la Gu°a de Usuario de PGP. Si quiere hacer la
traducci¢n a su idioma materno, p¢ngase primero en contacto conmigo para
obtener la informaci¢n m†s reciente y las gu°as generales, y para ver si
ya se ha traducido a ese idioma. Colin Plumb (colin@nyx.cs.du.edu)
mantiene la colecci¢n completa de los lotes de idiomas.

Las pr¢ximas versiones de PGP pueden tener que cambiar el formato de
datos para mensajes, firmas, claves y anillos, para proporcionar nuevas
caracter°sticas importantes. Esto podr°a ocasionar problemas de
compatibilidad con esta versi¢n de PGP. Los pr¢ximos lanzamientos quiz†
vayan con utilidades de conversi¢n para las viejas claves, pero puede
que tenga que deshacerse de los mensajes creados con el antiguo PGP.

Si tiene acceso a Internet (sic), busque anuncios sobre los nuevos
lanzamientos de PGP en los grupos de noticias "sci.crypt" y el propio
grupo de PGP, "alt.security.pgp". Hay tambiÇn una lista de correo
llamada info-pgp, para usuarios que no tienen acceso al grupo de
noticias "alt.security.pgp". Info-pgp tiene como moderador a Hugh
Miller, y puede suscribirse escribiÇndole una carta a info-pgp-
request@lucpul.it.luc.edu. Incluya su nombre y direcci¢n Internet. Si
quiere saber c¢mo obtener PGP, Hugh puede enviarle una lista de
establecimientos FTP y telÇfonos de BBSs. Puede encontrarle tambiÇn en
hmiller@lucpul.it.luc.edu


Controles de exportaci¢n
=-----------------------

El Gobierno ha declarado ilegal en muchos casos la exportaci¢n de buena
tecnolog°a criptogr†fica, y esto podr°a incluir a PGP. Consideran este
tipo de programa como municiones. Viene determinado por vol†tiles
directivas del Departamento de Estado, no por leyes fijas. No voy a
exportar este programa desde los EEUU ni Canad†, en los casos en que sea
ilegal seg£n las directivas del Departamento de Estado de los EEUU, y no
asumo responsabilidad alguna si otras personas lo exportan por su
cuenta.

Si usted vive fuera de los EEUU o Canad†, le aconsejo que no quebrante
las directivas del Departamento de Estado obteniendo PGP de una fuente
en los EEUU. Dado que miles de usuarios locales lo obtuvieron despuÇs de
su publicaci¢n inicial, de alguna manera sali¢ de los EEUU y se extendi¢
ampliamente por el extranjero, como semillas de diente de le¢n flotando
en el viento. Si PGP ha conseguido llegar a su pa°s, no creo que
infrinja la ley norteamericana de exportaci¢n si lo obtiene de una
fuente fuera de los EEUU.

A algunos observadores legales con los que he hablado les parece que los
creadores de los controles de exportaci¢n de los EEUU nunca previeron
que se fuesen a aplicar alguna vez a un programa criptogr†fico libre y
gratuito que se ha publicado y distribuido a los cuatro vientos. Es
dif°cil imaginar a un abogado estadounidense intentar iniciar un
procedimiento judicial contra alguien por la "exportaci¢n" de un
programa publicado libremente en los EEUU. Hasta donde saben las
personas con las que he hablado, nunca se ha hecho, a pesar de haber
numerosos ejemplos de transgresiones de las normas de exportaci¢n. No
soy un abogado ni estoy dando consejo legal-- s¢lo intento indicar lo
que parece sentido com£n.

Desde la versi¢n 2.0 de PGP, el punto de lanzamiento del programa ha
estado fuera de los EEUU, en ordenadores accesibles p£blicamente en
Europa. En cada lanzamiento, lo vuelven a enviar a los EEUU y lo ponen
en ordenadores de acceso p£blico en los EEUU activistas de la intimidad
PGP de otros pa°ses. Hay algunas restricciones en los EEUU respecto a la
importaci¢n de municiones, pero no tengo conocimiento de ning£n caso en
el que esto se haya aplicado por importar programas criptogr†ficos a los
EEUU. Imagino que una acci¢n legal de ese tipo ser°a un buen
espect†culo.

Algunos gobiernos imponen castigos severos a cualquier persona dentro
del pa°s que simplemente utilice comunicaciones encriptadas. En algunos
pa°ses podr°an incluso fusilarle por ello. Pero si vive en un pa°s as°,
quiz† necesite PGP todav°a m†s.


Grupos pol°ticos relacionados con la inform†tica
================================================

PGP es un programa pol°tico. Parece apropiado mencionar aqu° algunos
grupos activistas relacionados con ordenadores. Se proporcionan detalles
completos sobre estos grupos, y c¢mo unirse a ellos, en un documento
aparte en el lote de lanzamiento de PGP.

La Electronic Frontier Foundation (EFF) se fund¢ en julio de 1990 para
asegurar la libertad de expresi¢n por medios digitales, con Çnfasis
particular en la aplicaci¢n de los principios encarnados en la
Constituci¢n norteamericana y el Acta de Derechos a la comunicaci¢n
basada en ordenadores. Puede encontrarlos en: Electronic Frontier
Foundation, 238 Main Street, Cambridge, MA 02142, EEUU.

La League for Programming Freedom (LPF) es una organizaci¢n de base
formada por catedr†ticos, estudiantes, hombres de negocios,
programadores y usuarios, dedicada a devolver la libertad de escribir
programas. Consideran las patentes sobre algoritmos de ordenador
perjudiciales para la industria de programaci¢n de los EEUU. Puede
encontrarlos en el n£mero 1617-433-7071 o enviando correo Internet a
lpf@uunet.uu.net

Computer Professionals For Social Responsibility (CPSR) da la capacidad
a los profesionales y usuarios de ordenadores para abogar por el uso
responsable de la tecnolog°a de la informaci¢n, y permite a todos
aquellos que utilizan tecnolog°a de ordenadores participar en debates
p£blicos sobre el impacto de los ordenadores en la sociedad. Puede
encontrarlos en:  1-415-322-3778 en Palo Alto, direcci¢n de correo-E
cpsr@csli.stanford.edu.

Para obtener m†s informaci¢n sobre estos grupos, consulte el documento
que va en el lote de lanzamiento de PGP.


Lecturas recomendadas
=====================

1)  Dorothy Denning, "Cryptography and Data Security", Addison-Wesley,
    Reading, MA 1982
2)  Dorothy Denning, "Protecting Public Keys and Signature Keys",
    IEEE Computer, Feb 1983
3)  Martin E. Hellman, "The Mathematics of Public-Key Cryptography,"
    Scientific American, Agosto 1979
4)  Steven Levy, "Crypto Rebels", WIRED, May/June 1993, page 54.
    (Es un art°culo "obligatorio" sobre PGP y otros temas relacionados)

Otras lecturas
==============

5)  Ronald Rivest, "The MD5 Message Digest Algorithm",
    MIT Laboratory for Computer Science, 1991
6)  Xuejia Lai, "On the Design and Security of Block Ciphers",
    Institute for Signal and Information Processing, ETH-Zentrum,
    Zurich, Switzerland, 1992
7)  Xuejia Lai, James L. Massey, Sean Murphy, "Markov Ciphers and
    Differential Cryptanalysis", Advances in Cryptology- EUROCRYPT'91
8)  Philip Zimmermann, "A Proposed Standard Format for RSA
    Cryptosystems", Advances in Computer Security, Vol III, editado por
    Rein Turn, Artech House, 1988
9)  Bruce Schneier, "Applied Cryptography: Protocols, Algorithms, and
    Source Code in C", John Wiley & Sons, 1993 (sale en Noviembre)
10) Paul Wallich, "Electronic Envelopes", Scientific American, Feb
    1993, pages 30-32. (Un art°culo sobre PGP)


Para ponerse en contacto con el autor
=====================================

Philip Zimmermann se encuentra en:

Boulder Software Engineering
3021 Eleventh Street
Boulder, Colorado 80304  EEUU
Tel +1-303-541-0140 (voz o FAX)  (10:00am - 7:00pm Mountain Time)
Internet:  prz@acm.org



ApÇndice A: D¢nde obtener PGP
=============================

Esta secci¢n describe c¢mo obtener el programa libre y gratuito PGP de
criptograf°a de clave p£blica (Pretty Good Privacy) desde un
establecimiento de FTP an¢nima en Internet, o por otras fuentes.

PGP tiene una completa gesti¢n de claves, un esquema de encriptaci¢n
h°brido RSA/convencional, res£menes de mensajes para firmas digitales,
compresi¢n de datos antes de encriptar y un buen dise§o ergon¢mico. PGP
es r†pido y tiene muchas prestaciones, con una excelente documentaci¢n.
El c¢digo fuente es gratuito.

PGP utiliza el criptosistema RSA, reclamado por una patente de los EEUU
en poder de una empresa llamada Public Key Partners. Los usuarios de PGP
fuera de los EEUU deben tomar nota de que no hay patente de RSA fuera de
los EEUU. TambiÇn deben tener en cuenta que hay leyes de exportaci¢n en
EEUU y Canad† que proh°ben a sus habitantes exportar programas
criptogr†ficos como este. Si usted vive fuera de los EEUU, probablemente
no estÇ transgrediendo las leyes de exportaci¢n de los EEUU si lo
obtiene fuera de ese pa°s.

A continuaci¢n se muestra una peque§a muestra de los sitios en los que
se supone que tienen PGP, en junio de 1993. No se garantiza que la
informaci¢n sea correcta. Algunos establecimientos en los EEUU han
retirado PGP alguna vez por temor a la intimidaci¢n legal de los due§os
de la patente de RSA.

Hay dos ficheros de archivo comprimidos en el lanzamiento est†ndar, con
el nombre del fichero derivado del n£mero de versi¢n. Para la versi¢n
2.3a de PGP, debe obtener pgp23A.zip, que contiene el ejecutable binario
DOS y la Gu°a de Usuario de PGP, y puede obtener opcionalmente
pgp23src.zip, que contiene el c¢digo fuente. Estos ficheros pueden
descomprimirse con la utilidad compartida PKUNZIP.EXE de descompresi¢n
de archivos, en sus versi¢n 1.10 o posterior. Los usuarios Unix que no
tengan implementaci¢n de UNZIP pueden encontrar el c¢digo fuente en el
fichero comprimido pgp21.src.tar.Z.

Un recordatorio:  Establezca el modo como binary o image para hacer la
transferencia FTP. Cuando reciba con kermit en el PC, indique modo
binario de 8 bits en ambos lados. He aqu° unos cuantos establecimientos
Internet que tienen PGP v°a FTP an¢nimo:

Finlandia:  nic.funet.fi  (128.214.6.100)
            Directorio: /pub/unix/security/crypt/

Italia:     ghost.dsi.unimi.it  (149.132.2.1)
            Directorio: /pub/security/

UK:         src.doc.ic.ac.uk
            Directorio: /computing/security/software/

Para aquellos que no tienen conexi¢n FTP con la red, nic.funet.fi
tambiÇn ofrece los ficheros por correo-E. Para obtener la versi¢n 2.3,
env°e el siguiente mensaje a mailserv@nic.funet.fi:

ENCODER uuencode
SEND pub/unix/security/crypt/pgp23src.zip
SEND pub/unix/security/crypt/pgp23.zip

Se depositar†n dos ficheros zip en su buz¢n, como (unos) 15 mensajes
agrupados, en unas 24 horas. Grabe y uudescodifique.

En los EEUU, PGP puede encontrarse en Dios sabe cu†ntas BBSs, demasiadas
para indicar aqu°. En cualquier caso, si no tiene ning£n n£mero a mano,
aqu° tiene algunos para probar.

La BBS GRAPEVINE en Little Rock, Arkansas, ha configurado una cuenta
especial para quien quiera recibir PGP gratis. El SYSOP es Jim Wenzel,
jim.wenzel@grapevine.lrk.ar.us. Estos son los n£meros de telÇfono, que
deben marcarse en este orden (esto es, el primero corresponde a la l°nea
de mayor velocidad):(1-501) 753-6859, (1-501) 753-8121, (1-501) 791-
0124. En la entrada, indique la siguiente informaci¢n:

name: PGP USER        ('PGP' es el nombre, 'USER' el apellido)
password: PGP

PGP tambiÇn est† disponible en Fidonet, una gran red informal de BBSs
basadas en PCs y conectadas por modems. Mire en las BBS de su †rea. Est†
en muchos BBS estadounidenses y del resto del mundo.

En Nueva Zelanda, intente esta BBS (supuestamente gratuita):
    Kappa Crucis:  +64 9 817-3714, -3725, -3324, -8424, -3094, -3393

Las distribuciones binaria y fuente de PGP est†n disponibles en la
biblioteca Canadian Broadcasting Corporation, abierta al p£blico. Tiene
sucursales en Toronto, Montreal y Vancouver. P¢ngase en contacto con Max
Allen, +1 416 205-6017 si tiene alguna pregunta.

Para obtener m†s informaci¢n sobre la implementaci¢n de PGP para Apple
Macintosh, Commodore Amiga o Atari ST, o para hacer cualquier otra
pregunta sobre c¢mo obtener PGP para otras plataformas, p¢ngase en
contacto con Hugh Miller en hmiller@lucpul.it.luc.edu.

He aqu° unas cuantas personas y sus direcciones electr¢nicas o n£meros
de telÇfono para obtener informaci¢n sobre la disponibilidad local de
PGP en algunos pa°ses:

Peter Gutmann                 Hugh Kennedy
pgut1@cs.aukuni.ac.nz         70042.710@compuserve.com
Nueva Zelanda                 Alemania, EC

Branko Lankester              Armando Ramos
lankeste@fwi.uva.nl           armando@clerval.org
+31 2159 42242                +34 48 50 30 98
Pa°ses Bajos                  Navarra, EC

Hugh Miller                   Colin Plumb
hmiller@lucpul.it.luc.edu     colin@nyx.cs.du.edu
(312) 508-2727                Toronto, Ontario, Canada
EEUU

Jean-loup Gailly
jloup@chorus.fr
Francia, EC



-----BEGIN PGP SIGNATURE-----
Version: 2.3a

iQCVAgUBLRYI0nAkxuOrd59xAQFjWgP/U8F1s88ySqehzZONQG+1XA4SWZoMQltR
x66+voNTjD/cgZTF58yw+QJ3E1DJ8QkktLvRC/QBImhUHkG84PWovln3DcZ88uSJ
4BuKyckvu+fxY4IpnFm3TkC8cgZhotFiCygZtYDee/BPCpJzyAvLGehVrUEwyTSB
/UQMqNaKl9A=
=JyNI
-----END PGP SIGNATURE-----

# Ejemplo de fichero config.txt para PGP 2.3
# Se ignoran las l¡neas en blanco, al igual que todo lo que sigue a '#'.
# Las palabras clave no tienen en cuenta el tipo de letra.
# Todo lo que aparece aqu¡ puede anularse en la l¡nea de ordenes,
# indicando (por ejemplo) "+armor=on"

# MyName es una subcadena del identificador de usuario por omision para 
# las firmas.
# Si no se establece, PGP will utiliza la primera clave en el anillo
# (la £ltima creada) si no se indica el usuario con -u
# MyName = "A. N. Onimo"

# El idioma en el que aparecen los mensajes
#
# Idiomas disponibles:
#   en = ingl‚s (por omisi¢n), es = castellano, fr = franc‚s,
#   de = alem n, it = italiano, br = portugu‚s brasile¤o
#
# Idiomas no disponibles todav¡a:
#   esp = esperanto, lv = let¢n, lt3 = Lituano,
#   ru = ruso, nl = neerland‚s, fi = finland‚s,
#   hu = h£ngaro, no = noruego, pt = portugu‚s,
#   sv = sueco, da = dan‚s, is = island‚s,
#   zh = chino, ko = coreano, ar =  rabe, iw = hebreo,
#   el = griego, tr = turco, ja = japon‚s
#
# La mayor parte de estos c¢digos corresponden a los "Codigos para
# representacion de nombres de idioma" en dos letras ISO 639-1988
#
Language = es

# Conjunto de caracteres para mostrar mensajes y convertir ficheros de texto
# Si se establece esta variable como cp850, ascii o alt_codes, PGP realiza
# conversion entre conjuntos de caracteres si TextMode = on o si se indica
# la opcion -t cuando se encripta o se firma un fichero.
# 
# Conjuntos disponibles de caracteres:
#   lat¡n1, cp850, alt_codes, koi8, ascii
#
# En MSDOS con el conjunto est ndar debe utilizarse cp850 para una
# correcta traduccion de caracteres. Los conjuntos de caracteres rusos en
# MSDOS son normalmente alt_codes {codigos alt}.
#
# La eleccion por omision para CharSet es "noconv", que significa 'no
# convertir'
# Notese que noconv, lat¡n1, y koi8 se tratan como equivalentes.
#
CharSet = cp850

# TMP es el directorio para los ficheros borrador, normalmente un disco en RAM
# TMP = "e:\"     # Puede imponerse la variable de entorno TMP

# Pager es el programa de visualizaci¢n de ficheros que se utiliza para ver
# los mensajes con -m.  Si no se establece, o es "pgp", se utiliza el
# paginador incorporado.  El paginador establecido en config.txt anula la
# variable de entorno PAGER.
# Pager = "list"

# ArmorLines es el m ximo n£mero de l¡neas por paquete cuando se crea un
# fichero con armadura de transporte. Se establece en 0 para dejar entero
Armorlines = 720

# Quite el car cter de comentario para que ocurra lo que se describe

# Armor = on          # Se utiliza -a para armadura ASCII donde corresponda
# TextMode = on       # Se utiliza la opcion -t donde corresponda
# KeepBinary = on     # Al desencriptar no se borra el fichero .pgp
# verbose = on        # Mensajes verbosos para diagn¢stico
# compress = off      # Con off se elimina la compresi¢n, para depurar
# showpass = on       # Se muestra la contrase¤a al escribirla
# pkcs_compat = 0     # Utiliza formatos compatibles con versiones anteriores

# BakRing es la trayectoria a una copia de seguridad del anillo de claves
# secretas, normalmente en un diskette.  Las claves secretas se comparan con
# la copia de seguridad cuando se hace una comprobacion de anillo (pgp -kc)
# BakRing = "a:\secring.pgp"

# N£mero de firmas de completa confianza que se necesitan para hacer v lida
# una clave
Completes_Needed = 1

# N£mero de firmas de relativa confianza que se necesitan para hacer v lida
# una clave
Marginals_Needed = 2

# Cuantos niveles de referencias pueden ser referencias de otras.
Cert_Depth = 4

# TZFix son las horas que a¤adir a time() para obtener GMT, para los sellos
# de hora GMT.
# Como MSDOS asume que la hora local es US Pacific time, y corrige esa hora
# para conseguir GMT, haga TZFix=0 para California, -1 para Colorado,
# -2 para Chicago, -3 para NY, -8 para Londres, -9 para Amsterdam y Madrid.
# Sin embargo, si la variable de entorno TZ est  definida adecuadamente, puede
# dejarse TZFix=0.  Seguramente, los sistemas Unix no tengan que preocuparse
# de establecer TZFix.
# TZFix = 0

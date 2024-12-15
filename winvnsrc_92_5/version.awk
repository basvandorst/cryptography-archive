BEGIN { FS = "$" }
/Id:/ { x = match($2,/Exp/)
        y = substr($2,5,x-6)
        gsub(/ /,"   \t",y) 
        printf("\"%s\",\n", y)}

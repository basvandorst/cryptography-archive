{x[NR]=$0}
END{
   FS=" "
   endi=NR
   i=1
   split(x[i], champs)
   while (x[i] != ";;; --------------- COPYRIGHT ------------------------" \
	  && i<=endi)
     {
       i++
     }
   if (i<=endi)
     {
       while (x[i] != ";;; --------------- PRODUCT IDENTIFICATION -----------" \
             && i<=endi)
      {
         i++
         split(x[i], champs)
      }
      if (i<=endi)
      {
         split(x[i+1], champs)
         module="module.tex"
         print "\\section{Product identification}" >module
         print "\\begin{verbatim}" >>module
         print x[i] >>module
         i++
         split(x[i], champs)
         while (x[i]!=";;; --------------- END PRODUCT IDENTIFICATION -----")
         {
            print x[i] >>module
            i++
            split(x[i], champs)
         }
         print x[i] > module
         print "\\end{verbatim}" >>module
         print "\\section{Functions identification}" >>module
         while (i<=endi)
         {
            while (x[i] != \
                   ";;; --------------- Function identification ----------" && \
                   i<=endi)
            {
               i++
               split(x[i], champs)
            }
            if (i<=endi)
            {
               split(x[i+1], champs)
               fonction=champs[5]
               n=split(fonction, liste, "_")
               if (n>1)
               {
                  fonction=""
                  for (j=1; j<= n; j++)
                  {
                     fonction=fonction liste[j] "\\char'137"
                     if (liste[j+1]=="")
                     {
                        j++
                     }
                  }
               }
               print "\\subsection{Function {\\tt " fonction "}}" \
                                                                      >>module
               print "\\leavevmode" >> module
               print "\\begin{verbatim}" >>module
               print x[i] >>module
               i++
               split(x[i], champs)
               while (x[i] != \
                      ";;; --------------- End function identification ----" \
                      && i<=endi)
               {
                  print x[i] >>module
                  i++
                  split(x[i], champs)
               }
               print x[i] > module
               print "\\end{verbatim}" >>module
               i++
               split(x[i], champs)
            }
         }
      }
   }
}

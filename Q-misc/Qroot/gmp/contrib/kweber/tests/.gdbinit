define pp
printf "(%x...%x...%x,%d),(%x...%x...%x,%d)\n", u[ul-1],u[ul/2],u[0],ul,v[vl-1],v[vl/2],v[0],vl
end

define gcduv
set variable ul = _mpn_bingcd(u,u,ul,v,vl)
pp
end

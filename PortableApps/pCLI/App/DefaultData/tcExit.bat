@Rem If option PathExt is set, and .BTM is not listed in %PathExt
@Rem   this 4Start.BAT is executed.
@Rem   There is no LoadBtm in TCC/LE
@Rem Let's restart 4Start.BTM

@*If exist %@Quote[%@Path[%_BatchName]%@Name[%_BatchName].btm] @*%@Quote[%@Path[%_BatchName]%@Name[%_BatchName].btm]

@*Pause Ooops! I'm back. Probably %@Quote[%@Path[%_BatchName]%@Name[%_BatchName].btm] does not exist

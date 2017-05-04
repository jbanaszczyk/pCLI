@*If IsInternal LoadBtm *LoadBtm ON

:: If option PathExt is set, and .BTM is not listed in %PathExt
::   this 4Start.BAT is executed.

@*If exist %@Quote[%@Path[%_BatchName]%@Name[%_BatchName].btm] @*%@Quote[%@Path[%_BatchName]%@Name[%_BatchName].btm]

@*Pause Ooops! I'm back. Probably %@Quote[%@Path[%_BatchName]%@Name[%_BatchName].btm] does not exist

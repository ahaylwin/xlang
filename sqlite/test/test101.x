from xlang_sqlite import sqlite

xlang_addr ="some where in usa"
cantor_addr ="cantor way in usa"

company_list = None

pushWritepad(sqlite)
#create a database:example and a table: commpany

%USE example;
%CREATE TABLE company(\
   ID INT PRIMARY KEY     NOT NULL,\
   NAME           TEXT    NOT NULL,\
   AGE            INT     NOT NULL,\
   ADDRESS        CHAR(50),\
   Score          REAL\
);
%INSERT INTO company VALUES(1,'XLang',2,${xlang_addr},100);
%INSERT INTO company VALUES(2,'CantorAI',1,${cantor_addr},99.9);
% company_list = SELECT * FROM company;
re =company_list.fetch()
%DROP TABLE company;

popWritepad()

print(re)

print("end")


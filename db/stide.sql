/*  sql command used to create stide.db */
CREATE TABLE words_(word VARCHAR(100));
.import muw.dict words_
CREATE TABLE words(id INTEGER PRIMARY KEY AUTOINCREMENT, word VARCHAR(100));
INSERT INTO words(word) SELECT * FROM words_;
DROP TABLE words_;

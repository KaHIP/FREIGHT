CREATE TABLE IF NOT EXISTS "times"(
  "index" INTEGER,
  "k" INTEGER,
  "seed" INTEGER,
  "graph" TEXT,
  "algorithm" TEXT,
  "time_gp" REAL,
  "time_io" REAL
);

CREATE TABLE IF NOT EXISTS "results"(
  "index" INTEGER,
  "k" INTEGER,
  "seed" INTEGER,
  "graph" TEXT,
  "algorithm" TEXT,
  "obj_func" REAL,
  "graph_edges" INTEGER
);

CREATE TABLE IF NOT EXISTS "connectivities"(
  "index" INTEGER,
  "k" INTEGER,
  "seed" INTEGER,
  "graph" TEXT,
  "algorithm" TEXT,
  "obj_func" REAL,
  "pins" INTEGER
);

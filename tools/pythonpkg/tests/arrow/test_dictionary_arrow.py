import duckdb
try:
    import pyarrow as pa
    import pyarrow.parquet
    import numpy as np
    can_run = True
except:
    can_run = False

def compare_results(query):
    true_answer = duckdb.query(query).fetchall()
    t = duckdb.query(query).arrow()
    from_arrow = duckdb.from_arrow_table(duckdb.query(query).arrow()).fetchall()
    assert true_answer == from_arrow

class TestArrowDictionary(object):
    def test_dictionary(self,duckdb_cursor):
        if not can_run:
            return
        indices = pa.array([0, 1, 0, 1, 2, 1, 0, 2])
        dictionary = pa.array([10, 100, None])
        dict_array = pa.DictionaryArray.from_arrays(indices, dictionary)
        arrow_table = pa.Table.from_arrays([dict_array],['a'])
        rel = duckdb.from_arrow_table(arrow_table)
        
        assert rel.execute().fetchall() == [(10,), (100,), (10,), (100,), (None,), (100,), (10,), (None,)]
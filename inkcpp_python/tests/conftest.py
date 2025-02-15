import pytest
import os
import sys
import inkcpp_py as ink

@pytest.fixture(scope='session', autouse=True)
def inklecate_cmd():
    res = os.getenv("INKLECATE")
    if res is None or res == "":
        return "inklecate"
    return res


def extract_paths(tmpdir):
    def res(ink_source):
        name = os.path.splitext(os.path.basename(ink_source))[0]
        return (
        name,
            list(map(lambda x: tmpdir + ("/" + name + x), [".bin", ".tmp"])) + ["./inkcpp_test/ink/" + ink_source]
        )
    return res

@pytest.fixture(scope='session', autouse=True)
def story_path(tmpdir_factory):
    tmpdir = tmpdir_factory.getbasetemp()
    # tmpdir = os.fsencode('/tmp/pytest')
    return {name: files 
        for (name, files) in map(extract_paths(tmpdir), 
            filter(
                lambda file: os.path.splitext(file)[1] == ".ink", 
                os.listdir("./inkcpp_test/ink/")))}
            
@pytest.fixture(scope='session', autouse=True)
def assets(story_path, inklecate_cmd):
    res = {}
    for (name, files) in story_path.items():
        if not os.path.exists(files[0]):
            if not os.path.exists(files[1]):
                os.system('{} -o {} {}'.format(inklecate_cmd, files[1], files[2]))
            ink.compile_json(str(files[1]), str(files[0]))
        res[name] = ink.Story.from_file(str(files[0]))
    return res

@pytest.fixture(scope='session', autouse=True)
def generate():
    def g(asset):
        store = asset.new_globals()
        return [
            asset,
            store,
            asset.new_runner(store),
        ]
    return g

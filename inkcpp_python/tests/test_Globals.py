import inkcpp_py as ink
import pytest


class TestGlobals:
    def test_reading_globals(self, assets, generate):
        [story, globals, runner] = generate(assets['GlobalStory'])

        assert runner.getline() == "My name is Jean Passepartout, but my friend's call me Jackie. I'm 23 years old.\n"
        assert runner.getline() == "Foo:23\n"

        val = globals.age
        assert val.type == ink.Value.Int32 and str(val) == '23'
        val = globals.friendly_name_of_player
        assert val.type == ink.Value.String and str(val) == 'Jackie'

    def test_writing_globals(self, assets, generate):
        [story, globals, runner] = generate(assets['GlobalStory'])
        globals.age = ink.Value(30)
        globals.friendly_name_of_player = ink.Value("Freddy")

        
        assert runner.getline() == "My name is Jean Passepartout, but my friend's call me Freddy. I'm 30 years old.\n"
        assert runner.getline() == "Foo:30\n"

        val = globals.age
        assert val.type == ink.Value.Int32 and str(val) == '30'
        val = globals.friendly_name_of_player
        assert val.type == ink.Value.String and str(val) == 'Freddy'
        

    def test_invalid_operations(self, assets, generate):
        [story, globals, runner] = generate(assets['GlobalStory'])

        with pytest.raises(KeyError):
            val = globals.foo
        with pytest.raises(KeyError):
            globals.foo = ink.Value(0)
        with pytest.raises(KeyError):
            globals.age = ink.Value('foo')

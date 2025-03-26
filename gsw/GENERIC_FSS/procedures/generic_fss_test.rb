require 'cosmos'
require 'cosmos/script'
require 'mission_lib.rb'

class FSS_LPT < Cosmos::Test
  def setup
    
  end

  def test_lpt
    start("tests/generic_fss_lpt_test.rb")
  end

  def teardown

  end
end

class FSS_CPT < Cosmos::Test
  def setup
      
  end

  def test_cpt
    start("tests/generic_fss_cpt_test.rb")
  end

  def teardown

  end
end

class Generic_fss_Test < Cosmos::TestSuite
  def initialize
      super()
      add_test('FSS_CPT')
      add_test('FSS_LPT')
  end

  def setup
  end
  
  def teardown
  end
end
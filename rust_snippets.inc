//{"///",    "////////////////////////////////", 32, 32},

// control based
{"ifel", "if  {\n\n} else {\n\n}", 2, 6},
{"iflet", "if let Some() =  {\n\n}", 12, 16},

// for variants
{"fori",     "for i in 0.. {\n\n}\n", 12, 15},
{"forj",     "for j in 0.. {\n\n}\n", 12, 15},
{"forx",     "for x in 0.. {\n\n}\n", 12, 15},
{"fory",     "for y in 0.. {\n\n}\n", 12, 15},
{"forn",     "for   in  {\n\n}\n", 4, 9},

// y and x versions of the same things
{"fny",     "pub fn (&mut self) {\n\n}\n", 7, 17},
{"fnx",     "pub fn (&self) {\n\n}\n", 7, 14},
{"defy",     "impl Default for  {\nfn default() -> Self {\nSelf {\n\n}\n}\n}\n", 17, 50},
{"defx",     "Default::default()", 18, 0},
{"newy",     "impl  {\npub fn new() -> Self {\nSelf {\n\n..Default::default()\n}\n}\n}\n", 5, 38},
{"appy", "app: &mut App, ", 15, 0},
{"appx", "app: &App, ", 11, 0},
{"mody", "mod ;\npub use self::::*;\n", 4, 20},
{"imply",     "impl  {\npub fn () {\n\n}\n}\n", 5, 15},
{"implx",     "impl  for  {\npub fn () {\n\n}\n}\n", 5, 10},
{"cratey", "use crate::::*;", 11, 14},
{"cratex", "crate::::();", 7, 9},

// default ones that you rarely write or deviate from
{"print", "println!();", 9, 0},
{"match", "match  {\n => \n_ => {}\n}\n", 6, 9},
{"struct",     "pub struct  {\n\n}\n", 11, 14},
{"enum",     "pub enum  {\n\n}\n", 9, 12},
{"trait",     "pub trait  {\n\n}\n", 10, 13},

// special ones
{"derive", "#[derive()]", 9, 0},
{"allow", "#[allow()]", 8, 0},

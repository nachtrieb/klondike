# set global variables that define a card deck

suit_label <- c("C","D","H","S")
face_label <- c("A","2","3","4","5","6","7","8","9","T","J","Q","K")

deck_id <- seq(1,52) # unique card id, AC..KS
deck_suit <- c(rep(1,13),rep(2,13),rep(3,13),rep(4,13)) # suits 1..4 correspond to C,D,H,S
deck_face <- rep(seq(1,13),4)
deck_color <- c(rep(0,13),rep(1,13),rep(1,13),rep(0,13))
deck_label <- paste0(face_label[deck_face],suit_label[deck_suit])

aces <- deck_id[deck_face == 1] # id of the aces
king <- deck_id[deck_face == 13] # id of the kings

piles <- seq(5,11) # the column numbers of the piles

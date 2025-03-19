int FindTone(short Key, short ScaleNumber){
	short Chrom[] = { 1 }; // 1 
	short Minor[] = { 2, 1, 2, 2, 1, 2, 2 }; // 2
	short Major[] = { 2, 2, 1, 2, 2, 2, 1 }; // 3
	short Dorian[] = { 2, 1, 2, 2, 2, 1, 2 }; // 4
	short MixoLydian[] = { 2, 2, 1, 2, 2, 1, 2 }; // 5
	short Lydian[] = { 2, 2, 2, 1, 2, 2, 1 }; // 6
	short WholeTone[] = { 2 }; // 7
	short Phrygian[] = { 1, 2, 2, 2, 1, 2, 2 }; // 8
	short Locrian[] = { 1, 2, 2, 1, 2, 2 , 2}; // 9
	short Aeolian[] = { 2, 1, 2, 2, 1, 2, 2 }; // 10
	short Byzantine[] = { 1, 3, 1, 2, 1, 3, 1 }; // 14
	short Diminished[] = { 2, 1, 2, 1, 2, 1, 2, 1 }; // 11
	short Minpen[] = { 3, 2, 2, 3, 2 }; // 12
	short Majpen[] = { 2, 2, 3, 2, 3 }; // 13
	short Enigmatic[] = { 1, 3, 2, 2, 2, 1, 1 }; // 15
	short Prometheus[] = { 2, 2, 2, 3, 1, 2 }; // 16
	short Geez[] = { 2, 1, 2, 2, 1, 2, 2 }; // 17
	short Hindu[] = { 2, 2, 1, 2, 1, 2, 2 }; // 18
	short Octatonic[] = { 2, 2, 1, 2, 1, 1, 2, 1 }; // 19
	short Maqams[] = { 1, 3, 1, 2, 1, 3, 1 }; // 20
	short HarmonicMinor[] = { 2, 1, 2, 2, 1, 3, 1 }; // 21
	short TriTone[] = { 1, 3, 1, 1, 3, 1 }; // 22

	short Scale = 0;
	short Tone = 0;
	short Step = 0;
	short Counter = Key;

	while(Counter != 0){
		switch(ScaleNumber){
			case 0:
				if (Step == 1) Step = 0;
				Scale = Chrom[Step];
				Tone = Tone + Scale;
				break;
			case 1:
				if (Step == 7) Step = 0;
				Scale = Minor[Step];
				Tone = Tone + Scale;
				break;
			case 2:
				if (Step == 7) Step = 0; 
				Scale = Major[Step]; 
				Tone = Tone + Scale;
				break;
			case 3:
				if (Step == 7) Step = 0;
				Scale = Dorian[Step];
				Tone = Tone + Scale;
				break;
			case 4:
				if (Step == 7) Step = 0;
				Scale = MixoLydian[Step];
				Tone = Tone + Scale;
				break;
			case 5:
				if (Step == 7) Step = 0;
				Scale = Lydian[Step];
				Tone = Tone + Scale;
				break;
			case 6:
				if (Step == 1) Step = 0;
				Scale = WholeTone[Step];
				Tone = Tone + Scale;
			break;
				case 7:
				if (Step == 7) Step = 0;
				Scale = Phrygian[Step];
				Tone = Tone + Scale;
				break;
			case 8:
				if (Step == 7) Step = 0;
				Scale = Locrian[Step];
				Tone = Tone + Scale;
				break;
			case 9:
				if (Step == 7) Step = 0;
				Scale = Aeolian[Step];
				Tone = Tone + Scale;
				break;
			case 10:
				if (Step == 7) Step = 0;
				Scale = Byzantine[Step];
				Tone = Tone + Scale;
				break;
			case 11:
				if (Step == 8) Step = 0;
				Scale = Diminished[Step];
				Tone = Tone + Scale;
				break;
			case 12:
				if (Step == 5) Step = 0;
				Scale = Minpen[Step];
				Tone = Tone + Scale;
				break;
			case 13:
				if (Step == 5) Step = 0;
				Scale = Majpen[Step];
				Tone = Tone + Scale;
				break;
			case 14:
				if (Step == 7) Step = 0;
				Scale = Enigmatic[Step];
				Tone = Tone + Scale;
				break;
			case 15:
				if (Step == 6) Step = 0;
				Scale = Prometheus[Step];
				Tone = Tone + Scale;
				break;
			case 16:
				if (Step == 7) Step = 0;
				Scale = Geez[Step];
				Tone = Tone + Scale;
				break;
			case 17:
				if (Step == 7) Step = 0;
				Scale = Hindu[Step];
				Tone = Tone + Scale;
				break;
			case 18:
				if (Step == 8) Step = 0;
				Scale = Octatonic[Step];
				Tone = Tone + Scale;
				break;
			case 19:
				if (Step == 7) Step = 0;
				Scale = Maqams[Step];
				Tone = Tone + Scale;
				break;
			case 20:
				if (Step == 7) Step = 0;
				Scale = HarmonicMinor[Step];
				Tone = Tone + Scale;
				break;
			case 21:
				if (Step == 6) Step = 0;
				Scale = TriTone[Step];
				Tone = Tone + Scale;
				break;
			default: break;
		} 
		Step++;
		Counter--;
	} 
	return Tone;
}

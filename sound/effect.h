#ifndef SOUND_EFFECT_H
#define SOUND_EFFECT_H

#include <algorithm>
#include "../common/file_reader.h"

class Effect;
Effect* all_effects = NULL;

// Effect represents a set of sound files.
// We keep track of the minimum number found, the maximum number found, weather
// there is a file with no number, and if there are leading zeroes or not.
// Basically, it means that files can be numbered any which way, as long as it
// is consistent and there are no gaps in the numbering.
//
// Note that *ALL* sounds use this Effect class to keep track of the sounds.
// This means that you can for instance have hum1.wav, hum2.wav, hum3.wav, and
// every time the hum loops, it will randomly pick one of them.
class Effect {
  public:

  // This class represents a specific effect file. It's main purpose
  // is to be smaller than using the filename to identify the file.
  class FileID {
   public:
    FileID(const Effect* effect, int file) : effect_(effect), file_(file) {}
    FileID() : effect_(nullptr), file_(0) {}

    bool operator==(const FileID& other) const {
      return other.effect_ == effect_ && file_ == other.file_;
    }

    operator bool() const { return effect_ != nullptr; }

    void GetName(char *filename) {
      effect_->GetName(filename, file_);
    }

    const Effect* GetEffect() const { return effect_; }
    int GetFileNum() const { return file_; }

    // Maybe this should always use effect_->following_ ??
    FileID GetFollowing(Effect* effect) {
      if (effect_ &&
	  effect_->paired_ &&
	  effect_->files_found() == effect->files_found() &&
	  effect->selected_ == -1) {
	return FileID(effect, file_);
      } else {
	return effect->RandomFile();
      }
    }

   private:
    const Effect* effect_;
    int file_;
  };

  enum Extension {
    WAV,
    RAW,
    USL,
    BMP,
    PBM,
    Binary, // .BIN
    UNKNOWN,
  };

  enum class FilePattern : uint8_t {
    // No idea
    UNKNOWN,
    // NAMENNNN.WAV
    FLAT,
    // NAME/NAMENNNN.WAV
    SUBDIRS,
    // NAME/NNNN.WAV
    NONREDUNDANT_SUBDIRS,
  };

  enum class FileType : uint8_t {
    SOUND,
    IMAGE,
    UNKNOWN,
  };

  static FileType GetFileType(Extension x) {
    switch (x) {
      case WAV:
      case RAW:
      case USL:
	return FileType::SOUND;
      case BMP:
      case PBM:
      case Binary:
	return FileType::IMAGE;
      default:
	return FileType::UNKNOWN;
    }
  }

  static Extension IdentifyExtension(const char* filename) {
    if (endswith(".wav", filename)) return WAV;
    if (endswith(".raw", filename)) return RAW;
    if (endswith(".usl", filename)) return USL;
    if (endswith(".bmp", filename)) return BMP;
    if (endswith(".pbm", filename)) return PBM;
    if (endswith(".bin", filename)) return Binary;
    return UNKNOWN;
  }

  Effect(const char* name,
	 Effect* following = nullptr,
	 FileType file_type = FileType::SOUND) : name_(name) {
    following_ = following;
    file_type_ = file_type;
    next_ = all_effects;
    all_effects = this;
    reset();
  }

  void reset() {
    min_file_ = 20000;
    max_file_ = -1;
    digits_ = 0;
    unnumbered_file_found_ = false;
    file_pattern_ = FilePattern::UNKNOWN;
    ext_ = UNKNOWN;
    selected_ = -1;
    num_files_ = 0;
    directory_ = nullptr;
    volume_ = 100;
    paired_ = false;
  }

  bool Scan(const char *filename) {
    FilePattern type_if_found = FilePattern::FLAT;
    const char *rest = startswith(name_, filename);
    if (!rest) return false;
    if (*rest == '/') {
      const char *tmp = startswith(name_, rest + 1);
      if (tmp) {
        type_if_found = FilePattern::SUBDIRS;
        rest = tmp;
      } else {
        type_if_found = FilePattern::NONREDUNDANT_SUBDIRS;
        rest++;
      }
    }

    Extension ext = IdentifyExtension(filename);
    if (GetFileType(ext) != file_type_) return false;
    if (ext_ == UNKNOWN) {
      ext_ = ext;
    } else if (ext_ != ext) {
      // Different extension, ignore!
      return false;
    }

    int n = -1;
    if (*rest == '.' && strlen(rest) == 4) {
      unnumbered_file_found_ = true;
    } else {
      char *end;
      n = strtol(rest, &end, 10);
      if (n <= 0) return false;
      max_file_ = std::max<int>(max_file_, n);
      min_file_ = std::min<int>(min_file_, n);
      if (*rest == '0') {
        digits_ = end - rest;
      }
    }


    file_pattern_ = type_if_found;
    // STDOUT << "Counting " << filename << " as " << name_ << "\n";
    num_files_++;
    return true;
  }

  void Show() {
    if (files_found()) {
      STDOUT.print("Found ");
      STDOUT.print(name_);
      STDOUT.print(" files: ");
      if (min_file_ <= max_file_) {
        STDOUT.print(min_file_);
        STDOUT.print("-");
        STDOUT.print(max_file_);
        if (digits_) {
          STDOUT.print(" using ");
          STDOUT.print(digits_);
          STDOUT.print(" digits");
        }
        if (unnumbered_file_found_) {
          STDOUT.print(" + ");
        }
      }
      if (unnumbered_file_found_) {
        STDOUT.print("one unnumbered file");
      }
      switch (file_pattern_) {
        case FilePattern::UNKNOWN:
        case FilePattern::FLAT:
          break;
        case FilePattern::SUBDIRS:
          STDOUT.print(" in subdirs");
          break;
        case FilePattern::NONREDUNDANT_SUBDIRS:
          STDOUT.print(" in efficient subdirs");
      }
      if (files_found() != (size_t)num_files_) {
	STDOUT << " SOME FILES ARE MISSING! " << files_found() << " != " << num_files_;
      }
      STDOUT.print(" in ");
      STDOUT.print(directory_);
      STDOUT.println("");
    }
  }

  static void ShowAll() {
    for (Effect* e = all_effects; e; e = e->next_) {
      e->Show();
    }
    STDOUT.println("Done listing effects.");
  }

  size_t files_found() const {
    size_t ret = 0;
    if (min_file_ <= max_file_) {
      ret += max_file_ - min_file_ + 1;
    }
    if (unnumbered_file_found_) {
      ret ++;
    }
    return ret;
  }

  size_t get_min_file() const { return min_file_; }
	
  const char* get_directory() const { return directory_; }
	
  operator bool() const { return files_found() > 0; }

  void Select(int n) {
    selected_ = n;
  }
  void SelectNext() {
    selected_++;
    if (selected_ == (int)files_found()) selected_ = 0;
  }
  void SelectFloat(float value) {
    int f = files_found();
    int sel = clamp(floorf(f * value), 0, f - 1);
#ifdef NO_REPEAT_RANDOM
    for (int i = 0; i < 3 && (sel == selected_ || (sel == last_ && (rand() & 1))); i++) {
      sel = clamp(sel + 1 - (rand() & 2), 0, f - 1);
    }
    last_ = selected_;
#endif
    selected_ = sel;
  }
	
  Effect* GetFollowing() const {
    return following_;
  }

  void SetFollowing(Effect* following) {
    following_ = following;
  }

#ifdef NO_REPEAT_RANDOM
  int last_;
#endif

  FileID RandomFile() {
    int num_files = files_found();
    if (num_files < 1) {
      default_output->print("No sounds found: ");
      default_output->println(name_);
      return FileID();
    }
    int n;
    if (selected_ != -1) {
      n = selected_;
    } else {
      n = rand() % num_files;
#ifdef NO_REPEAT_RANDOM
      switch (num_files) {
      default:
	while (n == last_) n = rand() % num_files;
	break;
      case 2:
	if (n == last_) n = rand() % num_files;
      case 1:
	break;
      }
      last_ = n;
#endif
    }
    return FileID(this, n);
  }

  bool Play(char *filename) {
    FileID f = RandomFile();
    if (f == FileID()) return false;
    f.GetName(filename);
    return true;
  }

  // Get the name of a specific file in the set.
  void GetName(char *filename, int n) const {
    strcpy(filename, directory_);
    if (*directory_) strcat(filename, "/");
    strcat(filename, name_);
    switch (file_pattern_) {
      case FilePattern::UNKNOWN:
      case FilePattern::FLAT:
        break;
      case FilePattern::SUBDIRS:
        strcat(filename, "/");
        strcat(filename, name_);
        break;
      case FilePattern::NONREDUNDANT_SUBDIRS:
        strcat(filename, "/");
    }
    n += min_file_;
    // n can be max_file_ + 1, which means pick the file without digits.
    if (n <= max_file_) {
      char buf[12];
      itoa(n, buf, 10);
      char *j = filename + strlen(filename);
      int num_digits = strlen(buf);
      while (num_digits < digits_) {
        *j = '0';
        ++j;
        num_digits++;
      }
      memcpy(j, buf, strlen(buf) + 1);
    }

    switch (ext_) {
      case WAV: strcat(filename, ".wav"); break;
      case RAW: strcat(filename, ".raw"); break;
      case USL: strcat(filename, ".usl"); break;
      case BMP: strcat(filename, ".bmp"); break;
      case PBM: strcat(filename, ".pbm"); break;
      case Binary: strcat(filename, ".bin"); break;
      default: break;
    }

    default_output->print("Playing ");
    default_output->println(filename);
  }

  void SetPaired(bool i) { paired_ = i; }
  bool GetPaired() const { return paired_; }
  void SetVolume(uint8_t v) { volume_ = v; }
  uint8_t GetVolume() const { return volume_; }
  const char* GetName() const { return name_; }

  // Returns true if file was identified.
  static void ScanAll(const char *dir, const char* filename) {
    if (Effect::IdentifyExtension(filename) == Effect::UNKNOWN) {
      return;
    }

#if 0
    // TODO: "monitor scan" command?
    STDOUT.print("SCAN ");
    STDOUT.println(filename);
#endif
    for (Effect* e = all_effects; e; e = e->next_) {
      // This effect has already been found in a previous
      // directory, and it cannot be found in another directory.
      if (e->directory_ && e->directory_ != dir)
        continue;
      if (e->Scan(filename)) {
        e->directory_ = dir;
      }
    }
  }

  static void ScanCurrentDirectory() {
    LOCK_SD(true);
    for (Effect* e = all_effects; e; e = e->next_) {
      e->reset();
    }

    for (const char* dir = current_directory; dir; dir = next_current_directory(dir)) {
      STDOUT.print("Scanning sound font: ");
      STDOUT.print(dir);

#ifdef ENABLE_SERIALFLASH
      // Scan serial flash.
      SerialFlashChip::opendir();
      uint32_t size;
      char filename[128];
      while (SerialFlashChip::readdir(filename, sizeof(filename), size)) {
        const char* f = startswith(dir, filename);
        if (!f) continue;
        if (*f != '/') continue;
        ScanAll(f + 1, dir);
      }
#endif

#ifdef ENABLE_SD
      if (LSFS::Exists(dir)) {
        for (LSFS::Iterator iter(dir); iter; ++iter) {
          if (iter.isdir()) {
            char fname[128];
            strcpy(fname, iter.name());
            strcat(fname, "/");
            char* fend = fname + strlen(fname);
            for (LSFS::Iterator i2(iter); i2; ++i2) {
              strcpy(fend, i2.name());
              ScanAll(dir, fname);
            }
          } else {
            ScanAll(dir, iter.name());
          }
        }
	STDOUT.println(" done");
      } else {
	STDOUT.println(" NOT FOUND!");
#ifdef ENABLE_AUDIO
#if VERSION_MAJOR <= 3	
	if (strlen(dir) > 8) { // TODO: Check individual path segments
	  talkie.Say(talkie_font_directory_15, 15);
	  talkie.Say(talkie_too_long_15, 15);
	} else
#endif
        if (strlen(dir)) {
	  talkie.Say(talkie_font_directory_15, 15);
	  talkie.Say(talkie_not_found_15, 15);
	}
#endif   // ENABLE_AUDIO
      }
#endif   // ENABLE_SD
    }

    bool warned = false;
    for (Effect* e = all_effects; e; e = e->next_) {
      if (e->files_found() != (size_t)(e->num_files_)) {
	if (!warned) {
	  warned = true;
	  STDOUT.println("");
	  STDOUT.println("WARNING: This font seems to be missing some files!!");
	  talkie.Say(talkie_error_in_15, 15);
	  talkie.Say(talkie_font_directory_15, 15);
	}
	e->Show();
      }
    }
    LOCK_SD(false);
  }

  Effect* next_;
private:
  Effect* following_ = nullptr;

  // Minimum file number.
  int16_t min_file_;

  // Maximum file number.
  int16_t max_file_;

  // Number of files identified.
  int16_t num_files_;

  // Leading zeroes are used to make it this many digits.
  int8_t digits_;

  // Volume adjustment in percent.
  uint8_t volume_;

  // If true. there is an un-numbered file as well.
  bool unnumbered_file_found_ : 1;

  // If true, we play the same sound number is used when
  // we play the |folowing_| sound, unless one was specifically
  // selected.
  bool paired_ : 1;

  FilePattern file_pattern_ = FilePattern::UNKNOWN;

  // All files must start with this prefix.
  const char* name_;

  // If not -1, return this file.
  int16_t selected_;

  // All files must end with this extension.
  Extension ext_;

  // Image or sound?
  FileType file_type_;

  // The files for this effect are in this directory.
  const char* directory_;
};


#define EFFECT(X) Effect SFX_##X(#X)
#define EFFECT2(X, Y) Effect SFX_##X(#X, &SFX_##Y)
#define IMAGE_FILESET(X) Effect IMG_##X(#X, nullptr, Effect::FileType::IMAGE)

EFFECT(preon);
EFFECT(pstoff);

// Monophonic fonts
EFFECT(boot);     // also polyphonic
EFFECT(bladein);  // also polyphonic
EFFECT(bladeout);  // also polyphonic
EFFECT2(hum, hum);
EFFECT2(humm, humm);
EFFECT(swing);
EFFECT(poweron);
EFFECT2(poweroff, pstoff);
EFFECT2(pwroff, pstoff);
EFFECT(clash);
EFFECT(force);    // also polyphonic
EFFECT(stab);     // also polyphonic
#ifdef ENABLE_SPINS
EFFECT(spin);     // also polyphonic
#endif
EFFECT(blaster);
EFFECT2(lockup, lockup);
EFFECT(poweronf); // force poweron
EFFECT(font);     // also polyphonic
EFFECT(bgnlock);  // monophonic and polyphonic begin lock
EFFECT(endlock);  // Plecter endlock support, used for polyphonic name too

// Polyphonic fonts
EFFECT(blst);
EFFECT(clsh);
EFFECT2(in, pstoff);
EFFECT(out);
EFFECT2(lock, lock);
EFFECT(swng);
EFFECT(slsh);

// Looped swing fonts. (SmoothSwing V1/V2)
EFFECT2(swingl, swingl);  // Looped swing, LOW
EFFECT2(swingh, swingh);  // Looped swing, HIGH
EFFECT2(lswing, lswing);  // Looped swing, LOW (plecter naming)
EFFECT2(hswing, hswing);  // Looped swing, HIGH (plecter naming)

// Drag effect, replaces "lock/lockup" in drag mode if present.
EFFECT(bgndrag);
EFFECT2(drag, drag);
EFFECT(enddrag);

// Melt is like drag, but for door melting. Falls back to "drag".
EFFECT(bgnmelt);
EFFECT2(melt, melt);
EFFECT(endmelt);

// Lightning block is like "lockup", but for blocking force lightning.
// Falls back to standard lockup.
EFFECT(bgnlb);
EFFECT2(lb, lb);
EFFECT(endlb);

// Detonator effects
EFFECT(bgnarm);
EFFECT2(armhum, armhum);
EFFECT(endarm);
EFFECT(boom);

// Color change
EFFECT(color);
EFFECT(ccbegin);
EFFECT(ccend);
EFFECT(ccchange);

// Blaster effects
// hum, boot and font are reused from sabers and already defined.
EFFECT(bgnauto); // Doesn't exist in fonts, but I expect there may be use for autofire transitions
EFFECT2(auto,auto);
EFFECT(endauto); // Doesn't exist in fonts, but I expect there may be use for autofire transitions

EFFECT(blast); // Not to be confused with "blst" and "blaster" as blocking sounds in sabers

// battery low
EFFECT(lowbatt);	// battery low

// TODO: Optimize this and make it possible
// have the WAV reader use this.
class EffectFileReader : public FileReader {
public:
  bool Play(Effect* f) {
    do_open_ = false;
    Effect::FileID id = f->RandomFile();
    if (!id) {
      return false;
    }
    id.GetName(filename_);
    do_open_ = true;
    return true;
  }

  void Play(const char* filename) {
    do_open_ = false;
    strncpy(filename_, filename, sizeof(filename_));
    do_open_ = true;
  }

  // Returns true if we had been asked to open a file.
  // Check if open succeded or not by calling IsOpen()
  bool OpenFile() {
    if (!do_open_) return false;
    if (!OpenFast(filename_)) {
      default_output->print("File ");
      default_output->print(filename_);
      default_output->println(" not found.");
    }
    do_open_ = false;
    return true;
  }
private:
  volatile bool do_open_ = false;
  char filename_[128];
};

#endif
